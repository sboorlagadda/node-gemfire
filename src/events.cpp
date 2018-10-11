#include "events.hpp"
#include <nan.h>

using namespace v8;

namespace node_gemfire {

void emitEvent(const Local<Object> & emitter, const char * eventName) {
  Nan::HandleScope scope;

  static const int argc = 1;
  Local<Value> argv[argc] = { Nan::New(eventName).ToLocalChecked() };
  Nan::MakeCallback(emitter, "emit", argc, argv);
}

void emitEvent(const Local<Object> & emitter, const char * eventName, const Local<Value> & payload) {
  Nan::HandleScope scope;

  static const int argc = 2;
  Local<Value> argv[argc] = { Nan::New(eventName).ToLocalChecked(), payload };
  Nan::MakeCallback(emitter, "emit", argc, argv);
}

void emitError(const Local<Object> & emitter, const Local<Value> & error) {
  emitEvent(emitter, "error", error);
}

}  // namespace node_gemfire
