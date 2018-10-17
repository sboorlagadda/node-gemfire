#ifndef __EXCEPTIONS_HPP__
#define __EXCEPTIONS_HPP__

#include <v8.h>

#include <geode/GeodeCppCache.hpp>

namespace node_gemfire {

v8::Local<v8::Value> v8Error(const apache::geode::client::Exception& exception);
v8::Local<v8::Value> v8Error(
    const apache::geode::client::UserFunctionExecutionExceptionPtr&
        exceptionPtr);

void ThrowGemfireException(const apache::geode::client::Exception& e);

}  // namespace node_gemfire

#endif
