#ifndef __EXCEPTIONS_HPP__
#define __EXCEPTIONS_HPP__

#include <v8.h>

#include <geode/ExceptionTypes.hpp>
#include <geode/UserFunctionExecutionException.hpp>

namespace node_gemfire {

v8::Local<v8::Value> v8Error(const apache::geode::client::Exception& exception);
v8::Local<v8::Value> v8Error(
    const std::shared_ptr<
        apache::geode::client::UserFunctionExecutionException>& exceptionPtr);

[[noreturn]] void ThrowGemfireException(
    const apache::geode::client::Exception& e);

}  // namespace node_gemfire

#endif
