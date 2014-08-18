#ifndef __CACHE_HPP__

#include <v8.h>
#include <nan.h>
#include <node.h>
#include <gfcpp/Cache.hpp>

using namespace v8;

namespace node_gemfire {

class Cache : public node::ObjectWrap {
 public:
    static void Init(Handle<Object> exports);

    gemfire::CachePtr cachePtr;
 protected:
    ~Cache();
    static NAN_METHOD(New);
    static NAN_METHOD(ExecuteQuery);
    static NAN_METHOD(GetRegion);
    static void AsyncExecuteQuery(uv_work_t * request);
    static void AfterAsyncExecuteQuery(uv_work_t * request, int status);

    explicit Cache(gemfire::CachePtr cachePtr) : cachePtr(cachePtr) {}
};

class ExecuteQueryBaton {
 public:
    ExecuteQueryBaton(Handle<Function> callback,
                      gemfire::QueryPtr queryPtr) :
        queryPtr(queryPtr) {
      NanAssignPersistent(this->callback, callback);
    }

    ~ExecuteQueryBaton() {
      NanDisposePersistent(callback);
    }

    Persistent<Function> callback;
    gemfire::QueryPtr queryPtr;
    gemfire::SelectResultsPtr selectResultsPtr;
    gemfire::ExceptionPtr queryExceptionPtr;
    bool querySucceeded;
};

}  // namespace node_gemfire

#define __CACHE_HPP__
#endif
