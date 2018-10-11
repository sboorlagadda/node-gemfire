#ifndef __REGION_SHORTCUTS_HPP__
#define __REGION_SHORTCUTS_HPP__

#include <geode/RegionShortcut.hpp>
#include <string>

namespace node_gemfire {

apache::geode::client::RegionShortcut getRegionShortcut(const std::string & regionShortcutName);

extern apache::geode::client::RegionShortcut invalidRegionShortcut;

}  // namespace node_gemfire

#endif
