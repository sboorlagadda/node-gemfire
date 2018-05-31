#include <geode/GeodeCppCache.hpp>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "gemfire_worker.hpp"
#include "exceptions.hpp"

using namespace v8;

namespace node_gemfire {

void GemfireWorker::Execute() {
  try {
    ExecuteGemfireWork();
  } catch(apache::geode::client::Exception & exception) {
    SetError(exception.getName(), exception.getMessage());
    threwException = true;
  }
}

 void GemfireWorker::WorkComplete() {
    Nan::HandleScope scope;
    if(threwException){
        Nan::ThrowError(errorObject());
    } else if (ErrorMessage() == NULL){
      HandleOKCallback();
    } else {
      HandleErrorCallback();
    }
    delete callback;
    callback = NULL;
    threwException = false;
  }
  void GemfireWorker::SetError( const char * name, const char * message){
    errorName = name;
    SetErrorMessage(message);
  }

  Local<Value> GemfireWorker::errorObject() {
    Nan::EscapableHandleScope scope;
    Local<Object> err = Nan::Error(ErrorMessage()).As<v8::Object>();
    Nan::Set(err, Nan::New("name").ToLocalChecked(), Nan::New(errorName).ToLocalChecked());
    return scope.Escape(err);
  }
}  // namespace node_gemfire
