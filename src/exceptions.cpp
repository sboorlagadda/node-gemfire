#include "exceptions.hpp"

#include <nan.h>

using namespace v8;

namespace node_gemfire {

Local<Value> v8Error(const apache::geode::client::Exception& exception) {
  Nan::EscapableHandleScope scope;

  Local<Object> error = Nan::Error(exception.getMessage().c_str())->ToObject();
  Nan::Set(error, Nan::New("name").ToLocalChecked(),
           Nan::New(exception.getName()).ToLocalChecked());
  return scope.Escape(error);
}

Local<Value> v8Error(
    const std::shared_ptr<
        apache::geode::client::UserFunctionExecutionException>& exceptionPtr) {
  Nan::EscapableHandleScope scope;
  Local<Object> error =
      Nan::Error(exceptionPtr->getMessage().c_str())->ToObject();

  return scope.Escape(error);
}

[[noreturn]] void ThrowGemfireException(
    const apache::geode::client::Exception& e) {
  Nan::ThrowError(v8Error(e));
  throw nullptr;
}

}  // namespace node_gemfire
