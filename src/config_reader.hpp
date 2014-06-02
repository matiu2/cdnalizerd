/** Reads the /etc/cdnalzier.conf config file
 **/

#include <unordered_map>
#include <string>
#include <istream>
#include <ostream>

namespace cdnalizerd {

struct AuthDetails {
    std::string username;
    std::string apikey;
    std::string token;
};

struct ConfigEntry {
    AuthDetails& auth;
    std::string container;
    std::string remote_dir;
};

/// To enable unordered_map searching
size_t hash(const ConfigEntry& entry) {
    return std::hash<std::string> entry;
}

struct Config : std::unordered_map<std::string, ConfigEntry, hash> {
    std::vector<AuthDetails> authDetailsCollection; // Just to keep them alive for the ConfigEntries to reference
};

/** Reads in a config file.
 *  Every line can be of one of these 4 formats
 *  username=xx
 *  key=yyy
 *  container=zzz
 *  /local/path /remote/path
 *
 *  The settings lines affect all lines below them.
 *
 *  Further examples:
 *
 *  /local/path2 /remote/path2
 *  container=container2
 *  "/local/path with spaces" "/remote/path with spaces"
 *
 *  The last path will be in container2 instead of zzz, but will use the same auth settings
 *
 **/ 
Config read_config(istream& in) { 


}


}
