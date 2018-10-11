#include "exceptions.hpp"
#include <nan.h>

using namespace v8;
using namespace apache::geode::client;

namespace node_gemfire {

Local<Value> v8Error(const apache::geode::client::Exception & exception) {
  Nan::EscapableHandleScope scope;

  Local<Object> error  = Nan::Error(exception.getMessage())->ToObject();
  Nan::Set(error, Nan::New("name").ToLocalChecked(),Nan::New(exception.getName()).ToLocalChecked()); 
  return scope.Escape(error);
}

Local<Value> v8Error(const UserFunctionExecutionExceptionPtr & exceptionPtr) {
  Nan::EscapableHandleScope scope;
  Local<Object> error = Nan::Error(exceptionPtr->getMessage()->asChar())->ToObject();

  return scope.Escape(error);
}

void ThrowGemfireException(const apache::geode::client::Exception & e) {
  Nan::ThrowError(v8Error(e));
}

}  // namespace node_gemfire
