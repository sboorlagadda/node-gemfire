#include "functions.hpp"

#include <nan.h>
#include <v8.h>

#include <iostream>
#include <string>

#include <geode/FunctionService.hpp>

#include "conversions.hpp"
#include "dependencies.hpp"
#include "events.hpp"
#include "exceptions.hpp"
#include "streaming_result_collector.hpp"

using namespace v8;
using namespace apache::geode::client;

namespace node_gemfire {

class ExecuteFunctionWorker {
 public:
  ExecuteFunctionWorker(
      apache::geode::client::Execution executionPtr,
      const std::string &functionName,
      const std::shared_ptr<apache::geode::client::Cacheable>
          &functionArguments,
      const std::shared_ptr<apache::geode::client::CacheableVector>
          &functionFilter,
      const Local<Object> &emitterHandle)
      : resultStream(new ResultStream(this, (uv_async_cb)DataAsyncCallback,
                                      (uv_async_cb)EndAsyncCallback)),
        executionPtr(std::move(executionPtr)),
        functionName(functionName),
        functionArguments(functionArguments),
        functionFilter(functionFilter),
        ended(false),
        executeCompleted(false) {
    emitter.Reset(emitterHandle);
    request.data = reinterpret_cast<void *>(this);
  }

  ~ExecuteFunctionWorker() {
    emitter.Reset();
    delete resultStream;
  }

  static void Execute(uv_work_t *request) {
    ExecuteFunctionWorker *worker =
        static_cast<ExecuteFunctionWorker *>(request->data);
    worker->Execute();
  }

  static void ExecuteComplete(uv_work_t *request, int status) {
    ExecuteFunctionWorker *worker =
        static_cast<ExecuteFunctionWorker *>(request->data);
    worker->ExecuteComplete();
  }

  static void DataAsyncCallback(uv_async_t *async, int status) {
    ExecuteFunctionWorker *worker =
        reinterpret_cast<ExecuteFunctionWorker *>(async->data);
    worker->Data();
  }

  static void EndAsyncCallback(uv_async_t *async, int status) {
    ExecuteFunctionWorker *worker =
        reinterpret_cast<ExecuteFunctionWorker *>(async->data);
    worker->End();
  }

  void Execute() {
    try {
      if (functionArguments != nullptr) {
        executionPtr = executionPtr.withArgs(functionArguments);
      }

      if (functionFilter != nullptr) {
        executionPtr = executionPtr.withFilter(functionFilter);
      }

      auto resultCollectorPtr = std::shared_ptr<StreamingResultCollector>(
          new StreamingResultCollector(resultStream));
      executionPtr = executionPtr.withCollector(resultCollectorPtr);

      executionPtr.execute(functionName);
    } catch (const apache::geode::client::Exception &exception) {
      exceptionPtr = std::unique_ptr<apache::geode::client::Exception>(
          new apache::geode::client::Exception(exception));
    }
  }

  void ExecuteComplete() {
    if (exceptionPtr != nullptr) {
      Nan::HandleScope scope;
      emitError(Nan::New(emitter), v8Error(*exceptionPtr));
      ended = true;
    }

    executeCompleted = true;
    teardownIfReady();
  }

  void Data() {
    Nan::HandleScope scope;

    Local<Object> eventEmitter(Nan::New(emitter));

    auto resultsPtr = resultStream->nextResults();
    for (auto &&iterator : *resultsPtr) {
      Local<Value> result(v8Value(iterator));

      if (result->IsNativeError()) {
        emitError(eventEmitter, result);
      } else {
        emitEvent(eventEmitter, "data", result);
      }
    }

    resultStream->resultsProcessed();
  }

  void End() {
    Nan::HandleScope scope;

    emitEvent(Nan::New(emitter), "end");

    ended = true;
    teardownIfReady();
  }

  void teardownIfReady() {
    if (ended && executeCompleted) {
      delete this;
    }
  }

  uv_work_t request;

 private:
  ResultStream *resultStream;

  apache::geode::client::Execution executionPtr;
  std::string functionName;
  std::shared_ptr<apache::geode::client::Cacheable> functionArguments;
  std::shared_ptr<apache::geode::client::CacheableVector> functionFilter;
  Nan::Persistent<Object> emitter;
  std::unique_ptr<apache::geode::client::Exception> exceptionPtr;

  bool ended;
  bool executeCompleted;
};

Local<Value> executeFunction(Nan::NAN_METHOD_ARGS_TYPE info,
                             apache::geode::client::Cache &cachePtr,
                             apache::geode::client::Execution &executionPtr) {
  Nan::EscapableHandleScope scope;

  if (info.Length() == 0 || !info[0]->IsString()) {
    Nan::ThrowError("You must provide the name of a function to execute.");
    return scope.Escape(Nan::Undefined());
  }

  Local<Value> v8FunctionArguments;
  Local<Value> v8FunctionFilter;
  Local<Value> v8SynchronousFlag;
  bool synchronousFlag = false;

  if (info[1]->IsArray()) {
    v8FunctionArguments = info[1];
  } else if (info[1]->IsObject()) {
    Local<Object> optionsObject(info[1]->ToObject());
    v8FunctionArguments =
        optionsObject->Get(Nan::New("arguments").ToLocalChecked());
    v8FunctionFilter = optionsObject->Get(Nan::New("filter").ToLocalChecked());

    if (!v8FunctionFilter->IsArray() && !v8FunctionFilter->IsUndefined()) {
      Nan::ThrowError(
          "You must pass an Array of keys as the filter for "
          "executeFunction().");
      return scope.Escape(Nan::Undefined());
    }

    v8SynchronousFlag =
        optionsObject->Get(Nan::New("synchronous").ToLocalChecked());
    if (!v8SynchronousFlag->IsBoolean() && !v8SynchronousFlag->IsUndefined()) {
      Nan::ThrowError(
          "You must pass true or false for the synchronous option for "
          "executeFunction().");
      return scope.Escape(Nan::Undefined());
    } else if (!v8SynchronousFlag->IsUndefined()) {
      synchronousFlag = v8SynchronousFlag->ToBoolean()->Value();
    }
  } else if (!info[1]->IsUndefined()) {
    Nan::ThrowError(
        "You must pass either an Array of arguments or an options Object to "
        "executeFunction().");
    return scope.Escape(Nan::Undefined());
  }

  std::string functionName(*Nan::Utf8String(info[0]));

  std::shared_ptr<apache::geode::client::Cacheable> functionArguments;
  if (v8FunctionArguments.IsEmpty() || v8FunctionArguments->IsUndefined()) {
    functionArguments = nullptr;
  } else {
    functionArguments = gemfireValue(v8FunctionArguments, cachePtr);
  }

  std::shared_ptr<apache::geode::client::CacheableVector> functionFilter;
  if (v8FunctionFilter.IsEmpty() || v8FunctionFilter->IsUndefined()) {
    functionFilter = nullptr;
  } else {
    functionFilter = gemfireVector(v8FunctionFilter.As<Array>(), cachePtr);
  }

  if (synchronousFlag) {
    std::shared_ptr<apache::geode::client::CacheableVector> returnValue =
        CacheableVector::create();
    std::unique_ptr<apache::geode::client::Exception> exceptionPtr;
    apache::geode::client::Execution synchronousExceptionPtr;

    try {
      if (functionArguments != nullptr) {
        synchronousExceptionPtr = executionPtr.withArgs(functionArguments);
      }

      if (functionFilter != nullptr) {
        synchronousExceptionPtr =
            synchronousExceptionPtr.withFilter(functionFilter);
      }

      auto resultCollectorPtr = synchronousExceptionPtr.execute(functionName);

      auto resultsPtr = resultCollectorPtr->getResult();
      for (auto &&iterator : *resultsPtr) {
        returnValue->push_back(iterator);
      }
    } catch (const apache::geode::client::Exception &exception) {
      exceptionPtr = std::unique_ptr<apache::geode::client::Exception>(
          new apache::geode::client::Exception(exception));
    }
    if (returnValue->size() == 1) {
      return scope.Escape(v8Array(returnValue)->Get(0));
    } else {
      return scope.Escape(v8Array(returnValue));
    }
  } else {
    Local<Function> eventEmitterConstructor(
        Nan::New(dependencies)
            ->Get(Nan::New("EventEmitter").ToLocalChecked())
            .As<Function>());
    Local<Object> eventEmitter(eventEmitterConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext()).FromMaybe(Local<Object>()));

    ExecuteFunctionWorker *worker = new ExecuteFunctionWorker(
        std::move(executionPtr), functionName, functionArguments,
        functionFilter, eventEmitter);

    uv_queue_work(uv_default_loop(), &worker->request,
                  ExecuteFunctionWorker::Execute,
                  ExecuteFunctionWorker::ExecuteComplete);

    return scope.Escape(eventEmitter);
  }
}

}  // namespace node_gemfire
