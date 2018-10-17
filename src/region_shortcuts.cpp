#include "region_shortcuts.hpp"

#include <string>

using namespace apache::geode::client;

namespace node_gemfire {

RegionShortcut getRegionShortcut(const std::string& regionShortcutName) {
  if (regionShortcutName == "PROXY") {
    return RegionShortcut::PROXY;
  } else if (regionShortcutName == "CACHING_PROXY") {
    return RegionShortcut::CACHING_PROXY;
  } else if (regionShortcutName == "CACHING_PROXY_ENTRY_LRU") {
    return RegionShortcut::CACHING_PROXY_ENTRY_LRU;
  } else if (regionShortcutName == "LOCAL") {
    return RegionShortcut::LOCAL;
  } else if (regionShortcutName == "LOCAL_ENTRY_LRU") {
    return RegionShortcut::LOCAL_ENTRY_LRU;
  }

  return invalidRegionShortcut;
}

RegionShortcut invalidRegionShortcut = (RegionShortcut)-1;

}  // namespace node_gemfire
