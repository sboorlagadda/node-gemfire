#ifndef __EVENT_STREAM_HPP__
#define __EVENT_STREAM_HPP__

#include <uv.h>
#include <v8.h>

#include <cassert>
#include <string>
#include <vector>

#include <geode/CacheableBuiltins.hpp>
#include <geode/EntryEvent.hpp>

namespace node_gemfire {

class EventStream {
 public:
  explicit EventStream(void *target, uv_async_cb callback) {
    uv_mutex_init(&mutex);
    async.data = target;
    uv_mutex_lock(&mutex);
    uv_async_init(uv_default_loop(), &async, callback);
    uv_unref(reinterpret_cast<uv_handle_t *>(&async));
    uv_mutex_unlock(&mutex);
  }

  virtual ~EventStream() {
    uv_close(reinterpret_cast<uv_handle_t *>(&async), NULL);
    uv_mutex_destroy(&mutex);
  }

  class Event {
   public:
    Event(const std::string &eventName,
          const apache::geode::client::EntryEvent &event)
        : eventName(eventName),
          entryEvent(new apache::geode::client::EntryEvent(
              event.getRegion(), event.getKey(), event.getOldValue(),
              event.getNewValue(), event.getCallbackArgument(),
              event.remoteOrigin())) {}

    v8::Local<v8::Object> v8Object();
    std::string getName();
    std::shared_ptr<apache::geode::client::Region> getRegion();

   private:
    std::string eventName;
    std::shared_ptr<apache::geode::client::EntryEvent> entryEvent;
  };

  void add(Event *event);
  std::vector<Event *> nextEvents();

 private:
  static void teardownCallback(uv_work_t *request);
  static void afterTeardownCallback(uv_work_t *request, int status);
  void teardown();

  uv_mutex_t mutex;
  uv_async_t async;

  std::vector<Event *> eventVector;
};

}  // namespace node_gemfire

#endif
