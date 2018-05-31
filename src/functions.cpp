#include "functions.hpp"
#include <geode/FunctionService.hpp>
#include <nan.h>
#include <v8.h>
#include <string>
#include <iostream>
#include "conversions.hpp"
#include "dependencies.hpp"
#include "exceptions.hpp"
#include "events.hpp"
#include "streaming_result_collector.hpp"

using namespace v8;
using namespace apache::geode::client;

namespace node_gemfire {

class ExecuteFunctionWorker {
 public:
  ExecuteFunctionWorker(
      const ExecutionPtr & executionPtr,
      const std::string & functionName,
      const CacheablePtr & functionArguments,
      const CacheableVectorPtr & functionFilter,
      const Local<Object> & emitterHandle) :
    resultStream(
        new ResultStream(this,
                        (uv_async_cb) DataAsyncCallback,
                        (uv_async_cb) EndAsyncCallback)),
    executionPtr(executionPtr),
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

  static void Execute(uv_work_t * request) {
    ExecuteFunctionWorker * worker = static_cast<ExecuteFunctionWorker *>(request->data);
    worker->Execute();
  }

  static void ExecuteComplete(uv_work_t * request, int status) {
    ExecuteFunctionWorker * worker = static_cast<ExecuteFunctionWorker *>(request->data);
    worker->ExecuteComplete();
  }

  static void DataAsyncCallback(uv_async_t * async, int status) {
    ExecuteFunctionWorker * worker = reinterpret_cast<ExecuteFunctionWorker *>(async->data);
    worker->Data();
  }

  static void EndAsyncCallback(uv_async_t * async, int status) {
    ExecuteFunctionWorker * worker = reinterpret_cast<ExecuteFunctionWorker *>(async->data);
    worker->End();
  }

  void Execute() {
    try {
      if (functionArguments != NULLPTR) {
        executionPtr = executionPtr->withArgs(functionArguments);
      }

      if (functionFilter != NULLPTR) {
        executionPtr = executionPtr->withFilter(functionFilter);
      }

      ResultCollectorPtr resultCollectorPtr
        (new StreamingResultCollector(resultStream));
      executionPtr = executionPtr->withCollector(resultCollectorPtr);

      executionPtr->execute(functionName.c_str());
    } catch (const apache::geode::client::Exception & exception) {
      exceptionPtr = exception.clone();
    }
  }

  void ExecuteComplete() {
    if (exceptionPtr != NULLPTR) {
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

    CacheableVectorPtr resultsPtr(resultStream->nextResults());
    for (CacheableVector::Iterator iterator(resultsPtr->begin());
         iterator != resultsPtr->end();
         ++iterator) {
      Local<Value> result(v8Value(*iterator));

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
  ResultStream * resultStream;

  ExecutionPtr executionPtr;
  std::string functionName;
  CacheablePtr functionArguments;
  CacheableVectorPtr functionFilter;
  Nan::Persistent<Object> emitter;
  apache::geode::client::ExceptionPtr exceptionPtr;

  bool ended;
  bool executeCompleted;
};

Local<Value> executeFunction(Nan::NAN_METHOD_ARGS_TYPE info,
                             const CachePtr & cachePtr,
                             const ExecutionPtr & executionPtr) {
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
    v8FunctionArguments = optionsObject->Get(Nan::New("arguments").ToLocalChecked());
    v8FunctionFilter = optionsObject->Get(Nan::New("filter").ToLocalChecked());

    if (!v8FunctionFilter->IsArray() && !v8FunctionFilter->IsUndefined()) {
      Nan::ThrowError("You must pass an Array of keys as the filter for executeFunction().");
      return scope.Escape(Nan::Undefined());
    }

    v8SynchronousFlag = optionsObject->Get(Nan::New("synchronous").ToLocalChecked());
    if (!v8SynchronousFlag->IsBoolean() && !v8SynchronousFlag->IsUndefined()) {
      Nan::ThrowError("You must pass true or false for the synchronous option for executeFunction().");
      return scope.Escape(Nan::Undefined());
    } else if (!v8SynchronousFlag->IsUndefined()) {
      synchronousFlag = v8SynchronousFlag->ToBoolean()->Value();
    }
  } else if (!info[1]->IsUndefined()) {
    Nan::ThrowError("You must pass either an Array of arguments or an options Object to executeFunction().");
    return scope.Escape(Nan::Undefined());
  }

  std::string functionName(*Nan::Utf8String(info[0]));

  CacheablePtr functionArguments;
  if (v8FunctionArguments.IsEmpty() || v8FunctionArguments->IsUndefined()) {
    functionArguments = NULLPTR;
  } else {
    functionArguments = gemfireValue(v8FunctionArguments, cachePtr);
  }

  CacheableVectorPtr functionFilter;
  if (v8FunctionFilter.IsEmpty() || v8FunctionFilter->IsUndefined()) {
    functionFilter = NULLPTR;
  } else {
    functionFilter = gemfireVector(v8FunctionFilter.As<Array>(), cachePtr);
  }

  if (synchronousFlag) {
    CacheableVectorPtr returnValue = CacheableVector::create();
    apache::geode::client::ExceptionPtr exceptionPtr;
    ExecutionPtr synchronousExecutionPtr;

    try {
      if (functionArguments != NULLPTR) {
        synchronousExecutionPtr = executionPtr->withArgs(functionArguments);
      }

      if (functionFilter != NULLPTR) {
        synchronousExecutionPtr = synchronousExecutionPtr->withFilter(functionFilter);
      }

      ResultCollectorPtr resultCollectorPtr;
      resultCollectorPtr = synchronousExecutionPtr->execute(functionName.c_str());

      CacheableVectorPtr resultsPtr(resultCollectorPtr->getResult());
      for (CacheableVector::Iterator iterator(resultsPtr->begin());
           iterator != resultsPtr->end();
           ++iterator) {
        returnValue->push_back(*iterator);
      }
    } catch (const apache::geode::client::Exception & exception) {
      exceptionPtr = exception.clone();
    }
    if (returnValue->length() == 1) {
      return scope.Escape(v8Array(returnValue)->Get(0));
    } else {
      return scope.Escape(v8Array(returnValue));
    }
  } else {
    Local<Function> eventEmitterConstructor(Nan::New(dependencies)->Get(Nan::New("EventEmitter").ToLocalChecked()).As<Function>());
    Local<Object> eventEmitter(eventEmitterConstructor->NewInstance());

    ExecuteFunctionWorker * worker =
      new ExecuteFunctionWorker(executionPtr, functionName, functionArguments, functionFilter, eventEmitter);

    uv_queue_work(
        uv_default_loop(),
        &worker->request,
        ExecuteFunctionWorker::Execute,
        ExecuteFunctionWorker::ExecuteComplete);

    return scope.Escape(eventEmitter);
  }
}

}  // namespace node_gemfire
