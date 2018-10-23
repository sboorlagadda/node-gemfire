#include "select_results.hpp"

#include <sstream>

#include "conversions.hpp"

using namespace v8;
using namespace apache::geode::client;

namespace node_gemfire {

NAN_MODULE_INIT(SelectResults::Init) {
  Nan::HandleScope scope;

  Local<FunctionTemplate> constructorTemplate = Nan::New<FunctionTemplate>();

  constructorTemplate->SetClassName(Nan::New("SelectResults").ToLocalChecked());
  constructorTemplate->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(constructorTemplate, "toArray",
                          SelectResults::ToArray);
  Nan::SetPrototypeMethod(constructorTemplate, "each", SelectResults::Each);
  Nan::SetPrototypeMethod(constructorTemplate, "inspect",
                          SelectResults::Inspect);

  constructor().Reset(Nan::GetFunction(constructorTemplate).ToLocalChecked());

  Nan::Set(target, Nan::New("SelectResults").ToLocalChecked(),
           Nan::GetFunction(constructorTemplate).ToLocalChecked());
}

Local<Object> SelectResults::NewInstance(
    const std::shared_ptr<apache::geode::client::SelectResults>&
        selectResultsPtr) {
  Nan::EscapableHandleScope scope;
  const unsigned int argc = 0;
  Local<Value> argv[argc] = {};
  Local<Object> instance(
      Nan::New(SelectResults::constructor())->NewInstance(argc, argv));
  auto selectResults = new SelectResults(selectResultsPtr);
  selectResults->Wrap(instance);

  return scope.Escape(instance);
}

NAN_METHOD(SelectResults::ToArray) {
  Nan::HandleScope scope;

  auto selectResults = Nan::ObjectWrap::Unwrap<SelectResults>(info.Holder());
  auto selectResultsPtr = selectResults->selectResultsPtr;

  auto length = selectResultsPtr->size();

  auto array = Nan::New<Array>(length);
  for (decltype(length) i = 0; i < length; i++) {
    array->Set(i, v8Value((*selectResultsPtr)[i]));
  }
  info.GetReturnValue().Set(array);
}

NAN_METHOD(SelectResults::Each) {
  Nan::HandleScope scope;

  if (info.Length() == 0 || !info[0]->IsFunction()) {
    Nan::ThrowError("You must pass a callback to each()");
    info.GetReturnValue().Set(Nan::Undefined());
    return;
  }

  auto selectResults = Nan::ObjectWrap::Unwrap<SelectResults>(info.Holder());
  auto selectResultsPtr = selectResults->selectResultsPtr;

  Nan::Callback callback(Local<Function>::Cast(info[0]));

  for (auto&& iterator : *selectResultsPtr) {
    Local<Value> argv[1] = {v8Value(iterator)};
    callback(1, argv);
  }
  info.GetReturnValue().Set(info.Holder());
}

NAN_METHOD(SelectResults::Inspect) {
  Nan::HandleScope scope;

  auto selectResults = Nan::ObjectWrap::Unwrap<SelectResults>(info.Holder());
  auto selectResultsPtr = selectResults->selectResultsPtr;

  std::stringstream inspectStream;
  inspectStream << "[SelectResults size=" << selectResultsPtr->size() << "]";

  info.GetReturnValue().Set(Nan::New(inspectStream.str()).ToLocalChecked());
}

}  // namespace node_gemfire
