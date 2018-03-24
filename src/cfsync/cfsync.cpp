#include "cfsync.hpp"

#include "../processes/list.hpp"

#include <vector>
#include <iostream>

#include <boost/range.hpp>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

/// Lists files, directories and symlinks (without recursion), and sorted
inline std::vector<fs::path> listFiles(fs::path root) {
  std::vector<fs::path> result;
  for (auto &&path : fs::directory_iterator(root))
    result.push_back(path.path());
  std::sort(result.begin(), result.end());
  return result;
}

void cfsync(yield_context &yield, JobType jobType, ConfigEntry &entry) {
  // Account login information
  AccountCache accounts;
  // Connect to Rackspace and get an authentication token
  fillSingleAccountCache(yield, entry, accounts);
  auto found = accounts.find(entry.username);
  assert(found != accounts.end());
  Rackspace &rs = found->second;
  URL url(rs.getURL(entry.region, entry.snet));
  // Walk the files (ordered by inode, not alphabetically)
  auto remoteChunks =
      cdnalizerd::processes::detailedListContainer(yield, rs, entry);
  // Do the work
  // For each of the remote files (already sorted)
  std::cout << "Remote: \n";
  for (const auto &remoteList : remoteChunks) {
    for (const auto &remoteLine :
         boost::make_iterator_range(remoteList.begin(), remoteList.end())) {
      // For now, just print out the local and remote dirs
      std::cout << remoteLine << '\n';
    }
  }
  std::cout << "\nLocal: \n";
  // For each directory
  for (auto &file : listFiles(entry.local_dir)) {
    std::cout << file << '\n';
  }
  /*
  switch (jobType) {
  case upload: {
  }
  case download: {
  }
  case server_copy: {
  }
  case local_copy: {
  }
  };
  */
}

