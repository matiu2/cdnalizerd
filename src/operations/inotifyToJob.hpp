#pragma once

#include "../Status.hpp"

namespace cdnalizerd {

Job inotifyEventToJob(Status &status, inotify::Event &&event) {
  const ConfigEntry &entry = status.watchToConfig[event.watch().handle()];
  // This may be a move or uplod operation
  Operation uploadOperation = entry.move ? Move : Upload;
  // We'll use this in several places below
  std::string serverSource =
      joinPaths(joinPaths(*entry.container, entry.remote_dir),
                unJoinPaths(entry.local_dir, event.path()));

  if (event.wasSaved() && !event.isDir()) {
    // Simple upload / move
    return Job{entry, uploadOperation, event.path(),
               joinPaths(*entry.container,
                         unJoinPaths(entry.local_dir, event.path()))};
  } else if (event.wasMovedFrom()) {
    // This may be a server side rename (copy + delete)
    if (event.destination) {
      const ConfigEntry &dest =
          status.watchToConfig[event.destination->watch().handle()];
      std::string serverDestinaton =
          joinPaths(joinPaths(*dest.container, dest.remote_dir),
                    unJoinPaths(dest.local_dir, event.destination->path()));

      if ((*dest.username == *entry.username) &&
          ((*dest.region == *entry.region))) {
        // If the source and the destination are in the same region and use the
        // same login, server side copy, then delete.
        return Job {
          entry, Copy, serverSource, serverDestinaton,
              std::shared_ptr<Job>(
                  new Job{Job{entry, Delete, serverSource}})
        };
      } else {
        // The file is being moved from one account to another or from one
        // region to another
        // We'll just upload to one account, then delete on the other
        const ConfigEntry &dest =
            status.watchToConfig[event.destination->watch().handle()];
        return Job{entry, uploadOperation, event.path(), serverDestinaton,
                   std::shared_ptr<Job>(
                       new Job{Job{dest, Delete, serverSource}})};
      }
    } else {
      // This file was moved out of our known directory space, delete it from
      // the server
      return Job{entry, Delete, serverSource};
    }
  } else if (event.wasMovedTo()) {
    // This file was moved from some unknown source to our space. Upload it to
    // the server
    return Job{entry, uploadOperation, event.path(), serverSource};
  } else if (event.wasDeleted()) {
    return Job{entry, Delete, serverSource};
  }
  return Job{};
}

  
} /* cdnalizerd  */ 
