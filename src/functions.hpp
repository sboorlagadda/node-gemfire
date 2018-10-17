#ifndef __FUNCTIONS_HPP__
#define __FUNCTIONS_HPP__

#include <nan.h>
#include <v8.h>

#include <geode/Cache.hpp>
#include <geode/Execution.hpp>

namespace node_gemfire {

v8::Local<v8::Value> executeFunction(
    Nan::NAN_METHOD_ARGS_TYPE info, apache::geode::client::Cache& cachePtr,
    apache::geode::client::Execution& executionPtr);

}  // namespace node_gemfire

#endif
