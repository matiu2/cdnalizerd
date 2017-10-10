#include "mainProcess.hpp"

#include "../inotify.hpp"
#include "login.hpp"
#include "syncAllDirectories.hpp"
#include "../WorkerManager.hpp"
#include "../jobs/upload.hpp"
#include "../jobs/delete.hpp"
#include "../logging.hpp"
#include "../exception_tags.hpp"

#include <boost/log/trivial.hpp>
#include <boost/filesystem.hpp>
#include <boost/exception/enable_error_info.hpp>
#include <boost/throw_exception.hpp>

namespace cdnalizerd {

namespace processes {

namespace fs = boost::filesystem;

constexpr int maskToFollow =
    IN_CREATE | IN_CLOSE_WRITE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO;

// Maps an inotify cookie to the event that last had that cookie
using Cookies = std::map<uint32_t, inotify::Event>;

// Maps inotify watch handles to config entries
using WatchToConfig = std::map<uint32_t, ConfigEntry>;

void watchNewDirectory(inotify::Instance &inotify, WatchToConfig &watchToConfig,
                       const ConfigEntry &entry, const char *path) {
  LOG_S(1) << "Adding inotify watch for: " << path;
  // Starts watching a directory
  inotify::Watch &watch = inotify.addWatch(path, maskToFollow);
  watchToConfig[watch.handle()] = entry;
}

void recursivelyWatchDirectory(inotify::Instance &inotify,
                               WatchToConfig &watchToConfig,
                               const ConfigEntry &entry, const char *path) {
  watchNewDirectory(inotify, watchToConfig, entry, entry.local_dir.c_str());
  // Recurse to all sub directories
  for (auto d = fs::recursive_directory_iterator(path); d != decltype(d)();
       ++d) {
    if (fs::is_directory(*d))
      watchNewDirectory(inotify, watchToConfig, entry,
                        d->path().native().c_str());
  }
}

/// Reads our configuration object and creates all the inotify watches needed
void createINotifyWatches(inotify::Instance &inotify,
                          WatchToConfig &watchToConfig, const Config &config) {
  for (const ConfigEntry &entry : config.entries())
    recursivelyWatchDirectory(inotify, watchToConfig, entry, entry.local_dir.c_str());
}

void watchForFileChanges(yield_context yield, const Config &config) {
  try {
    LOG_S(INFO) << "Creating inotify watches..." << std::endl;
    // Setup
    inotify::Instance inotify(yield);
    // Maps inotify watch handles to config entries
    std::map<uint32_t, ConfigEntry> watchToConfig;
    createINotifyWatches(inotify, watchToConfig, config);

    // Account login information
    AccountCache accounts;
    login(yield, accounts, config);

    WorkerManager workers;

    syncAllDirectories(yield, accounts, config, workers);

    /// Holds file move operations that are waiting for a pair
    std::map<uint32_t, inotify::Event> cookies;

    LOG_S(5) << "Waiting for file events" << std::endl;
    while (true) {
      inotify::Event event = inotify.waitForEvent();
      LOG_S(5) << "Got an inotify event: " << event << std::endl;

      // Get the job data ready
      const ConfigEntry &entry = watchToConfig[event.watch().handle()];
      auto found = accounts.find(entry.username);
      if (found == accounts.end())
        BOOST_THROW_EXCEPTION(
            boost::enable_error_info(std::runtime_error(
                "All Rackspace accounts should be initialized "
                "by the time this is called"))
            << err::username(*entry.username));
      Rackspace &rs = found->second;
      fs::path localFile(event.path());
      URL url(rs.getURL(*entry.region, entry.snet));
      auto worker = workers.getWorker(url.whole(), rs);
      std::string localRelativePath(
          fs::relative(event.path(), entry.local_dir).string());

      // If the file was closed and may have been written, upload if checksum is
      // different
      if (event.wasClosed()) {
        auto size = fs::file_size(localFile);
        LOG_S(5) << "File was closed for writing: " << localFile.native() << " "
                 << size << " bytes";
        if (size > 0)
          worker->addJob(jobs::makeConditionalUploadJob(
              localFile,
              url / *entry.container / entry.remote_dir / localRelativePath));
      } else if (event.wasIgnored()) {
          LOG_S(9) << "Removing inotify watch for deleted directory";
          auto found = watchToConfig.find(event.watch().handle());
          // We should have already made a note of this watch and be tracking it
          assert(found != watchToConfig.end());
          watchToConfig.erase(found);
          inotify.removeWatch(event.watch());
      } else if (event.wasDeleted()) {
        LOG_S(9) << "Got delete event: " << event.path();
        if (!event.isDir()) {
          LOG_S(9) << "Creating delete job";
          worker->addJob(jobs::makeRemoteDeleteJob(
              url / *entry.container / entry.remote_dir / localRelativePath));
        }
      } else if (event.wasCreated()) {
        // ... if a directory was created, add a watch to it
        if (event.isDir())
          watchNewDirectory(inotify, watchToConfig, entry,
                            event.path().c_str());
        // TODO: Sometimes files are created with > zero bytes. Check the file
        // size; if it's > 0, upload it
      }
      /*
      // If it's a move event, find its pair
      if (event.cookie) {
        auto found = cookies.find(event.cookie);
        if (found != cookies.end()) {
          if (event.wasMovedTo())
            std::swap(event, found->second);
          if (event.wasMovedFrom())
            event.destination.reset(new
      inotify::Event(std::move(found->second)));
          else {
            /// TODO: Log a runtime error here, only move events should have
            /// cookies
            cookies.erase(found);
          }
        } else {
          // The event's partner hasn't been found yet.
          // Store it in cookies; start another worker that'll wait 1 second,
      then
          // launch it as a delete if it's still there
          // TODO: Launch the cleanup worker
          continue; // Wait for its partner to appear
        }
      }
      // Now we have a whole event...
      if (event.wasCreated()) {
        // ... if a directory was created, add a watch to it
        fs::path path = event.path();
        if (fs::is_directory(path)) {
          inotify.addWatch(path.c_str(), maskToFollow);
          continue;
        }
        // TODO: Sometimes files are created with > zero bytes. Check the file
        // size; if it's > 0, upload it
      }
      if (event.wasClosed()) {
        // File was closed; could have been written.
        // Upload if newer than the remote

      }

      // If this event is a move and has a destionation..
      if (event.wasMovedFrom()) {
        if (event.destination) {
          // This may be a server side rename (copy + delete)
          const ConfigEntry &dest =
              watchToConfig[event.destination->watch().handle()];
          // The original job should be a server side copy
          worker->addJob(makeServerSideMoveJob(
              remoteURL, rs.getURL(*dest.region, dest.snet) / *dest.container /
                             dest.remote_dir /
                             unJoinPaths(dest.local_dir, event.path())));
        } else {
          // This file was moved out of our known directory space, delete it
          // from the server
          job.operation = SDelete;
          job.dest.clear();
        }
        // Put it in the right queue
        auto worker = workers.getWorker(job.workerURL(), rs);
        worker->addJob(std::move(job));
      } else if (event.wasDeleted()) {
        job.operation = SDelete;
        job.source.swap(job.dest);
        job.dest.clear();
      }
        */
    }
  } catch (boost::exception &e) {
    LOG_S(ERROR) << "watchForFileChanges failed: "
                 << boost::diagnostic_information(e, true);
  } catch (std::exception &e) {
    LOG_S(ERROR) << "watchForFileChanges failed (std::exception): "
                 << ": " << boost::diagnostic_information(e, true) << std::endl;
  } catch (...) {
    LOG_S(ERROR) << "watchForFileChanges failed (unkown exception): "
                 << boost::current_exception_diagnostic_information(true)
                 << std::endl;
  }
}

} /* processes  */ 
} /* cdnalizerd  */ 
