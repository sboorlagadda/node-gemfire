#ifndef __REGION_EVENT_LISTENER_HPP__
#define __REGION_EVENT_LISTENER_HPP__

#include <geode/CacheListener.hpp>

namespace node_gemfire {

class RegionEventListener : public apache::geode::client::CacheListener {
 public:
  RegionEventListener() {}
  virtual void afterCreate(const apache::geode::client::EntryEvent & event);
  virtual void afterUpdate(const apache::geode::client::EntryEvent & event);
  virtual void afterDestroy(const apache::geode::client::EntryEvent & event);
};

}  // namespace node_gemfire

#endif
