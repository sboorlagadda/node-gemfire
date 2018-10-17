#ifndef __FUNCTIONS_HPP__
#define __FUNCTIONS_HPP__

#include <nan.h>
#include <v8.h>

#include <geode/Cache.hpp>

namespace node_gemfire {

v8::Local<v8::Value> executeFunction(
    Nan::NAN_METHOD_ARGS_TYPE info,
    const apache::geode::client::CachePtr& cachePtr,
    const apache::geode::client::ExecutionPtr& executionPtr);

}  // namespace node_gemfire

#endif
