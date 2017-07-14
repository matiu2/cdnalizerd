#include "AccountCache.hpp"

#include "Status.hpp"

#include <algorithm>
#include <list>

namespace cdnalizerd {

void fillAccountCache(yield_context &yield, const Config &config,
                      AccountCache &cache, std::function<void()> onDone) {
  for (const ConfigEntry& configEntry : config.entries()) {
    Rackspace &rs = cache[configEntry.username];
    REST api(rs.makeAPI(yield));
    if (rs.status() == Rackspace::Fresh)
      rs.login(*configEntry.username, *configEntry.apikey, api);
  }
  onDone();
}


}
