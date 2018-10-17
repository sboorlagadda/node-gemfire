#define NODE_GEMFIRE_VERSION "1.0.1"

#include <nan.h>
#include <v8.h>

#include <geode/CacheFactory.hpp>

#include "cache.hpp"
#include "cache_factory.hpp"
#include "dependencies.hpp"
#include "region.hpp"
#include "select_results.hpp"

using namespace v8;
using namespace apache::geode::client;

namespace node_gemfire {

NAN_METHOD(Initialize) {
  Nan::HandleScope scope;

  Local<Object> gemfire = Nan::New<Object>();

  Nan::DefineOwnProperty(gemfire, Nan::New("version").ToLocalChecked(),
                         Nan::New(NODE_GEMFIRE_VERSION).ToLocalChecked(),
                         static_cast<PropertyAttribute>(ReadOnly | DontDelete));

  Nan::DefineOwnProperty(
      gemfire, Nan::New("gemfireVersion").ToLocalChecked(),
      Nan::New(apache::geode::client::CacheFactory::getVersion())
          .ToLocalChecked(),
      static_cast<PropertyAttribute>(ReadOnly | DontDelete));

  node_gemfire::Cache::Init(gemfire);
  node_gemfire::Region::Init(gemfire);
  node_gemfire::SelectResults::Init(gemfire);
  node_gemfire::CacheFactory::Init(gemfire);

  dependencies.Reset(v8::Isolate::GetCurrent(), info[0]->ToObject());

  info.GetReturnValue().Set(gemfire);
}

}  // namespace node_gemfire

static void Initialize(Local<Object> exports) {
  Local<FunctionTemplate> initializeTemplate(
      Nan::New<FunctionTemplate>(node_gemfire::Initialize));
  exports->Set(Nan::New("initialize").ToLocalChecked(),
               initializeTemplate->GetFunction());
}

NODE_MODULE(gemfire, Initialize)
