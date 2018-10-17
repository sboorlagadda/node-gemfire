#include "event_stream.hpp"

#include <nan.h>

#include <string>
#include <vector>

#include "conversions.hpp"

using namespace v8;
using namespace apache::geode::client;

namespace node_gemfire {

void EventStream::add(Event *event) {
  uv_mutex_lock(&mutex);

  eventVector.push_back(event);
  uv_ref(reinterpret_cast<uv_handle_t *>(&async));
  uv_mutex_unlock(&mutex);

  uv_async_send(&async);
}

std::vector<EventStream::Event *> EventStream::nextEvents() {
  uv_mutex_lock(&mutex);

  std::vector<Event *> returnValue;

  for (auto &&event : eventVector) {
    returnValue.push_back(event);
  }

  eventVector.clear();

  uv_unref(reinterpret_cast<uv_handle_t *>(&async));
  uv_mutex_unlock(&mutex);

  return returnValue;
}

Local<Object> EventStream::Event::v8Object() {
  Nan::EscapableHandleScope scope;

  auto eventPayload = Nan::New<Object>();

  Nan::Set(eventPayload, Nan::New("key").ToLocalChecked(),
           v8Value(entryEvent->getKey()));
  Nan::Set(eventPayload, Nan::New("oldValue").ToLocalChecked(),
           v8Value(entryEvent->getOldValue()));
  Nan::Set(eventPayload, Nan::New("newValue").ToLocalChecked(),
           v8Value(entryEvent->getNewValue()));

  return scope.Escape(eventPayload);
}

std::string EventStream::Event::getName() { return eventName; }

std::shared_ptr<apache::geode::client::Region> EventStream::Event::getRegion() {
  return entryEvent->getRegion();
}

}  // namespace node_gemfire
