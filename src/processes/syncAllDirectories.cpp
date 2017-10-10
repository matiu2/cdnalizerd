#include "syncAllDirectories.hpp"

#include "../inotify.hpp"
#include "login.hpp"
#include "list.hpp"
#include "../Rackspace.hpp"
#include "../jobs/upload.hpp"
#include "../utils.hpp"
#include "../logging.hpp"

#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <set>
#include <iostream>

#include <boost/asio/deadline_timer.hpp>

namespace cdnalizerd {
namespace processes {

namespace fs = boost::filesystem;

void syncOneConfigEntry(yield_context yield, const Rackspace &rs,
                        const ConfigEntry &config, WorkerManager &workers) {
  LOG_S(5) << "Syncing config entry: " << config.username << " - "
           << config.region << " - " << (config.snet ? "snet" : "no snet")
           << std::endl;
  URL baseURL(rs.getURL(*config.region, config.snet));
  HTTPS conn(yield, baseURL.host);
  // Get iterators to our local files
  std::vector<fs::path> localFiles(
      fs::recursive_directory_iterator(config.local_dir),
      fs::recursive_directory_iterator());
  if (localFiles.size() == 0) {
    // There are no local files, so nothing to upload
    return;
  }
  // We need to grab all the local files and sort them because the normal
  // ordering is by inode number or something; but we want them to be
  // alphabetical, like what the cloud files server returns
  // TODO: Don't fill a vector with potentially MBs of files.
  // Solution 1: Make a smart iterator that iterates one directory, sorts it, then digs down.
  //      Con 1: You might have to go very very deep, so you may have to
  //       re-iterate over the contents again and again or cache a lot
  //      Con 2: You may have one directory with millions of files in it anyway
  // Solution 2: Download the entire contents of the remote container; if
  //             necessary into memeory mapped files holding vectors, then for
  //             each out of order file iteration, do a binary search.
  //      Con 1: How much work will it be to do all the binary searches ?
  //      Con 2: How much storage to hold all the remote container contents
  //
  //             I like this method (2) better though because it uses less in
  //             program resources
  //
  std::sort(localFiles.begin(), localFiles.end());
  auto local_iterator = localFiles.begin();
  auto local_end = localFiles.end();
  // Get files only in the remote path/prefix that we care about from the config
  auto remoteChunks  = JSONListContainer(yield, rs, config, true);
  for (const auto &remoteList : remoteChunks) {
    auto remote_iterator = remoteList.begin();
    auto remote_end = remoteList.end();
    while ((local_iterator != local_end) && (remote_iterator != remote_end)) {
      // We don't do anything with directories
      if (fs::is_directory(*local_iterator)) {
        ++local_iterator;
        continue;
      }
      // Compare the remote_dir with local file minus the config
      const std::string &remotePath(remote_iterator->at("name"));
      std::string remoteRelativePath(
          unJoinPaths(config.remote_dir, remotePath));
      std::string localRelativePath(
          "/"s + fs::relative(*local_iterator, config.local_dir).string());
      int diff = localRelativePath.compare(remoteRelativePath);
      auto upload = [&](){
        URL url(baseURL);
        auto worker = workers.getWorker(url.whole(), rs);
        worker->addJob(jobs::makeUploadJob(
            *local_iterator,
            url / *config.container / config.remote_dir / localRelativePath));
      };
      if (diff == 0) {
        using namespace boost::posix_time;
        // The local and remote files are the same one
        // check the modification time
        // Check the files' modification time in UTC against the
        // remoteEntry.at("last_modified') and "bytes" (for the size)
        ptime localTime(
            from_time_t(fs::last_write_time(*local_iterator)));
        std::string remote_raw = (*remote_iterator)["last_modified"];
        remote_raw[remote_raw.find('T')] = ' ';
        ptime remoteTime(time_from_string(remote_raw));
        if (localTime > remoteTime) {
          if (fs::file_size(*local_iterator) > 0)
            upload();
          // TODO: If the cloud file has data, and but locally the file is now
          // empty, depending on the mode, should we delete the cloud version ?
        }
        // Get the next pair of files
        ++local_iterator;
        ++remote_iterator;
      } else if (diff < 0) {
        // The local file is less than the remote file
        // The local file doesn't exist on the server and should be uploaded
        if (fs::file_size(*local_iterator) > 0)
          upload();
        // We need to get the next local file
        ++local_iterator;
      } else {
        // Local file has passed the remote file, so get the next remote_file
        ++remote_iterator;
      }
    }
  }
  // Upload any files left over
  while (local_iterator != local_end) {
    // The local file doesn't exist on the server and should be uploaded
    URL url(baseURL);
    auto worker = workers.getWorker(url.whole(), rs);
    std::string localRelativePath(
        fs::relative(*local_iterator, config.local_dir).string());
    worker->addJob(jobs::makeUploadJob(
        *local_iterator,
        url / *config.container / config.remote_dir / localRelativePath));
    ++local_iterator;
  }
}

struct CountSentry {
  size_t& count;
  boost::asio::deadline_timer& timer;
  CountSentry(size_t &count, boost::asio::deadline_timer &timer)
      : count(count), timer(timer) {
    ++count;
    LOG_S(5) << "Incremented count: " << count << std::endl;
  }
  ~CountSentry() {
    --count;
    LOG_S(5) << "Decremented count: " << count << std::endl;
    if (count == 0) {
      LOG_S(5) << "Decremented cancelling timer: " << count << std::endl;
      timer.cancel();
    }
  }
};

void syncAllDirectories(yield_context &yield, const AccountCache &accounts,
                        const Config &config, WorkerManager& workers) {
  // Sync all the entries in parallel, and block the main thread with a timer
  // until they're all done
  boost::asio::deadline_timer waitForSync(service(),
                                          boost::posix_time::minutes(10));
  size_t syncWorkers(0);
  for (const ConfigEntry &entry : config.entries()) {
    // Make a list of file information
    asio::spawn(service(), [
                               &rs = accounts.at(entry.username), &entry,
                               &workers, &syncWorkers, &waitForSync
    ](yield_context y) {
      LOG_S(5) << "Syncing config: " << entry.username << std::endl;
      CountSentry sentry(syncWorkers, waitForSync);
      syncOneConfigEntry(y, rs, entry, workers);
      LOG_S(5) << "Done Syncing config: " << entry.username << std::endl;
    });
  }

  // Wait for all the logins to finish
  try {
    waitForSync.async_wait(yield);
  } catch(boost::system::system_error& e) {
    // This should throw operation_aborted, because we cancel the timer once all the logins are complete
    if (e.code() != boost::asio::error::operation_aborted)
      throw e;
  }
}
    
} /* processes */ 
} /* cdnalizerd  */ 
