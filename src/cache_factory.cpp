#include "cache_factory.hpp"

#include <nan.h>
#include <node.h>
#include <v8.h>

#include <sstream>
#include <string>

#include <geode/Cache.hpp>
#include <geode/CacheFactory.hpp>
#include <geode/Region.hpp>

#include "cache.hpp"
#include "conversions.hpp"
#include "dependencies.hpp"
#include "exceptions.hpp"
#include "functions.hpp"
#include "gemfire_worker.hpp"
#include "region.hpp"
#include "region_shortcuts.hpp"

using namespace v8;

namespace node_gemfire {

NAN_MODULE_INIT(CacheFactory::Init) {
  Nan::HandleScope scope;
  v8::Local<v8::FunctionTemplate> constructorTemplate =
      Nan::New<v8::FunctionTemplate>(New);
  constructorTemplate->SetClassName(Nan::New("CacheFactory").ToLocalChecked());

  constructorTemplate->InstanceTemplate()->SetInternalFieldCount(1);

//  Nan::SetPrototypeMethod(constructorTemplate, "addLocator",
//                          CacheFactory::AddLocator);
//  Nan::SetPrototypeMethod(constructorTemplate, "addServer",
//                          CacheFactory::AddServer);
  Nan::SetPrototypeMethod(constructorTemplate, "create", CacheFactory::Create);
  Nan::SetPrototypeMethod(constructorTemplate, "set", CacheFactory::Set);
//  Nan::SetPrototypeMethod(constructorTemplate, "setFreeConnectionTimeout",
//                          CacheFactory::SetFreeConnectionTimeout);
//  Nan::SetPrototypeMethod(constructorTemplate, "setIdleTimeout",
//                          CacheFactory::SetIdleTimeout);
//  Nan::SetPrototypeMethod(constructorTemplate, "setLoadConditioningInterval",
//                          CacheFactory::SetLoadConditioningInterval);
//  Nan::SetPrototypeMethod(constructorTemplate, "setMaxConnections",
//                          CacheFactory::SetMaxConnections);
//  Nan::SetPrototypeMethod(constructorTemplate, "setMinConnections",
//                          CacheFactory::SetMinConnections);
  Nan::SetPrototypeMethod(constructorTemplate, "setPdxIgnoreUnreadFields",
                          CacheFactory::SetPdxIgnoreUnreadFields);
//  Nan::SetPrototypeMethod(constructorTemplate, "setPingInterval",
//                          CacheFactory::SetPingInterval);
//  Nan::SetPrototypeMethod(constructorTemplate, "setPRSingleHopEnabled",
//                          CacheFactory::SetPRSingleHopEnabled);
//  Nan::SetPrototypeMethod(constructorTemplate, "setReadTimeout",
//                          CacheFactory::SetReadTimeout);
//  Nan::SetPrototypeMethod(constructorTemplate, "setRetryAttempts",
//                          CacheFactory::SetRetryAttempts);
//  Nan::SetPrototypeMethod(constructorTemplate, "setServerGroup",
//                          CacheFactory::SetServerGroup);
//  Nan::SetPrototypeMethod(constructorTemplate, "setSocketBufferSize",
//                          CacheFactory::SetSocketBufferSize);
//  Nan::SetPrototypeMethod(constructorTemplate, "setStatisticInterval",
//                          CacheFactory::SetStatisticInterval);
//  Nan::SetPrototypeMethod(constructorTemplate, "setSubscriptionAckInterval",
//                          CacheFactory::SetSubscriptionAckInterval);
//  Nan::SetPrototypeMethod(constructorTemplate, "setSubscriptionEnabled",
//                          CacheFactory::SetSubscriptionEnabled);
//  Nan::SetPrototypeMethod(constructorTemplate,
//                          "setSubscriptionMessageTrackingTimeout",
//                          CacheFactory::SetSubscriptionMessageTrackingTimeout);
//  Nan::SetPrototypeMethod(constructorTemplate, "setSubscriptionRedundancy",
//                          CacheFactory::SetSubscriptionRedundancy);
//  Nan::SetPrototypeMethod(constructorTemplate, "setThreadLocalConnections",
//                          CacheFactory::SetThreadLocalConnections);
//  Nan::SetPrototypeMethod(constructorTemplate, "setUpdateLocatorListInterval",
//                          CacheFactory::SetUpdateLocatorListInterval);

  constructor().Reset(Nan::GetFunction(constructorTemplate).ToLocalChecked());

  Nan::Set(target, Nan::New("CacheFactory").ToLocalChecked(),
           Nan::GetFunction(constructorTemplate).ToLocalChecked());
}

NAN_METHOD(CacheFactory::New) {
  Nan::HandleScope scope;

  if (info.Length() != 2) {
    Nan::ThrowError(
        "You must call the CacheFactory with a properties file location and a "
        "callback.");
    info.GetReturnValue().Set(Nan::Undefined());
    return;
  }
  apache::geode::client::CacheFactory cacheFactoryPtr;
  if (info[0]->IsString()) {
    std::shared_ptr<apache::geode::client::Properties> gemfireProperties =
        apache::geode::client::Properties::create();
    gemfireProperties->load(*Nan::Utf8String(info[0]));
    cacheFactoryPtr = apache::geode::client::CacheFactory(gemfireProperties);
  }
  auto cacheFactory = new CacheFactory(cacheFactoryPtr);
  cacheFactory->callback = new Nan::Callback(Local<Function>::Cast(info[1]));
  cacheFactory->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

// NAN_METHOD(CacheFactory::AddLocator) {
//  Nan::HandleScope scope;
//  int argsLength = info.Length();
//  if(argsLength != 2 && info[0]->IsString() && info[1]->IsNumber()){
//    Nan::ThrowError("You must pass the address and the port of the locator.");
//    return;
//  }
//  auto cacheFactory =
//  Nan::ObjectWrap::Unwrap<CacheFactory>(info.This())->cacheFactory;
//  cacheFactory.addLocator(*Nan::Utf8String(info[0]),
//  Nan::To<int>(info[1]).FromJust()); info.GetReturnValue().Set(info.This());
//}
// NAN_METHOD(CacheFactory::AddServer) {
//  Nan::HandleScope scope;
//  int argsLength = info.Length();
//  if(argsLength != 2 && info[0]->IsString() && info[1]->IsNumber()){
//    Nan::ThrowError("You must pass the address and the port of the server.");
//    return;
//  }
//  auto cacheFactory =
//  Nan::ObjectWrap::Unwrap<CacheFactory>(info.This())->cacheFactory;
//  cacheFactory.addServer(*Nan::Utf8String(info[0]),
//  Nan::To<int>(info[1]).FromJust()); info.GetReturnValue().Set(info.This());
//}

NAN_METHOD(CacheFactory::Set) {
  Nan::HandleScope scope;
  int argsLength = info.Length();
  if (argsLength != 2 && info[0]->IsString() && info[1]->IsString()) {
    Nan::ThrowError("You must pass the property name and the property value.");
    return;
  }
  auto cacheFactory =
      Nan::ObjectWrap::Unwrap<CacheFactory>(info.This())->cacheFactory;
  cacheFactory.set(*Nan::Utf8String(info[0]), *Nan::Utf8String(info[1]));
  info.GetReturnValue().Set(info.This());
}

// NAN_METHOD(CacheFactory::SetFreeConnectionTimeout) {
//  Nan::HandleScope scope;
//  int argsLength = info.Length();
//  if(argsLength != 1 && info[0]->IsNumber()){
//    Nan::ThrowError("You must pass an integer value.");
//    return;
//  }
//  auto cacheFactory =
//  Nan::ObjectWrap::Unwrap<CacheFactory>(info.This())->cacheFactory;
//  cacheFactory.setFreeConnectionTimeout(Nan::To<int>(info[1]).FromJust());
//  info.GetReturnValue().Set(info.This());
//}
//
// NAN_METHOD(CacheFactory::SetIdleTimeout) {
//  Nan::HandleScope scope;
//  int argsLength = info.Length();
//  if(argsLength != 1 && info[0]->IsNumber()){
//    Nan::ThrowError("You must pass an integer value.");
//    return;
//  }
//  auto cacheFactory =
//  Nan::ObjectWrap::Unwrap<CacheFactory>(info.This())->cacheFactory;
//  cacheFactory.setIdleTimeout(Nan::To<int>(info[1]).FromJust());
//  info.GetReturnValue().Set(info.This());
//}
//
//
// NAN_METHOD(CacheFactory::SetLoadConditioningInterval) {
//  Nan::HandleScope scope;
//  int argsLength = info.Length();
//  if(argsLength != 1 && info[0]->IsNumber()){
//    Nan::ThrowError("You must pass an integer value.");
//    return;
//  }
//  auto cacheFactory =
//  Nan::ObjectWrap::Unwrap<CacheFactory>(info.This())->cacheFactory;
//  cacheFactory.setLoadConditioningInterval(Nan::To<int>(info[1]).FromJust());
//  info.GetReturnValue().Set(info.This());
//}
//
// NAN_METHOD(CacheFactory::SetMaxConnections) {
//  Nan::HandleScope scope;
//  int argsLength = info.Length();
//  if(argsLength != 1 && info[0]->IsNumber()){
//    Nan::ThrowError("You must pass an integer value.");
//    return;
//  }
//  auto cacheFactory =
//  Nan::ObjectWrap::Unwrap<CacheFactory>(info.This())->cacheFactory;
//  cacheFactory.setMaxConnections(Nan::To<int>(info[1]).FromJust());
//  info.GetReturnValue().Set(info.This());
//}
//
// NAN_METHOD(CacheFactory::SetMinConnections) {
//  Nan::HandleScope scope;
//  int argsLength = info.Length();
//  if(argsLength != 1 && info[0]->IsNumber()){
//    Nan::ThrowError("You must pass an integer value.");
//    return;
//  }
//  auto cacheFactory =
//  Nan::ObjectWrap::Unwrap<CacheFactory>(info.This())->cacheFactory;
//  cacheFactory.setMinConnections(Nan::To<int>(info[1]).FromJust());
//  info.GetReturnValue().Set(info.This());
//}

NAN_METHOD(CacheFactory::SetPdxIgnoreUnreadFields) {
  Nan::HandleScope scope;
  auto argsLength = info.Length();
  if (argsLength != 1 && info[0]->IsBoolean()) {
    Nan::ThrowError("You must pass an bool value.");
    return;
  }
  auto cacheFactory =
      Nan::ObjectWrap::Unwrap<CacheFactory>(info.This())->cacheFactory;
  cacheFactory.setPdxIgnoreUnreadFields(Nan::To<bool>(info[1]).FromJust());
  info.GetReturnValue().Set(info.This());
}

// NAN_METHOD(CacheFactory::SetPingInterval) {
//  Nan::HandleScope scope;
//  int argsLength = info.Length();
//  if(argsLength != 1 && info[0]->IsNumber()){
//    Nan::ThrowError("You must pass an long value.");
//    return;
//  }
//  auto cacheFactory =
//  Nan::ObjectWrap::Unwrap<CacheFactory>(info.This())->cacheFactory;
//  cacheFactory.setPingInterval(Nan::To<int64_t>(info[1]).FromJust());
//  info.GetReturnValue().Set(info.This());
//}

// NAN_METHOD(CacheFactory::SetPRSingleHopEnabled) {
//  Nan::HandleScope scope;
//  int argsLength = info.Length();
//  if(argsLength != 1 && info[0]->IsBoolean()){
//    Nan::ThrowError("You must pass an bool value.");
//    return;
//  }
//  auto cacheFactory =
//  Nan::ObjectWrap::Unwrap<CacheFactory>(info.This())->cacheFactory;
//  cacheFactory.setPRSingleHopEnabled(Nan::To<bool>(info[1]).FromJust());
//  info.GetReturnValue().Set(info.This());
//}

// NAN_METHOD(CacheFactory::SetReadTimeout) {
//  Nan::HandleScope scope;
//  int argsLength = info.Length();
//  if(argsLength != 1 && info[0]->IsNumber()){
//    Nan::ThrowError("You must pass an integer value.");
//    return;
//  }
//  auto cacheFactory =
//  Nan::ObjectWrap::Unwrap<CacheFactory>(info.This())->cacheFactory;
//  cacheFactory.setReadTimeout(Nan::To<int>(info[1]).FromJust());
//  info.GetReturnValue().Set(info.This());
//}
// NAN_METHOD(CacheFactory::SetRetryAttempts) {
//  Nan::HandleScope scope;
//  int argsLength = info.Length();
//  if(argsLength != 1 && info[0]->IsNumber()){
//    Nan::ThrowError("You must pass an integer value.");
//    return;
//  }
//  auto cacheFactory =
//  Nan::ObjectWrap::Unwrap<CacheFactory>(info.This())->cacheFactory;
//  cacheFactory.setRetryAttempts(Nan::To<int>(info[1]).FromJust());
//  info.GetReturnValue().Set(info.This());
//}
// NAN_METHOD(CacheFactory::SetServerGroup) {
//  Nan::HandleScope scope;
//  int argsLength = info.Length();
//  if(argsLength != 1 && info[0]->IsString()){
//    Nan::ThrowError("You must pass the server group name as a string.");
//    return;
//  }
//  auto cacheFactory =
//  Nan::ObjectWrap::Unwrap<CacheFactory>(info.This())->cacheFactory;
//  cacheFactory.setServerGroup(*Nan::Utf8String(info[0]));
//  info.GetReturnValue().Set(info.This());
//}
// NAN_METHOD(CacheFactory::SetSocketBufferSize) {
//  Nan::HandleScope scope;
//  int argsLength = info.Length();
//  if(argsLength != 1 && info[0]->IsNumber()){
//    Nan::ThrowError("You must pass an integer value.");
//    return;
//  }
//  auto cacheFactory =
//  Nan::ObjectWrap::Unwrap<CacheFactory>(info.This())->cacheFactory;
//  cacheFactory.setSocketBufferSize(Nan::To<int>(info[1]).FromJust());
//  info.GetReturnValue().Set(info.This());
//}
// NAN_METHOD(CacheFactory::SetStatisticInterval) {
//  Nan::HandleScope scope;
//  int argsLength = info.Length();
//  if(argsLength != 1 && info[0]->IsNumber()){
//    Nan::ThrowError("You must pass an integer value.");
//    return;
//  }
//  auto cacheFactory =
//  Nan::ObjectWrap::Unwrap<CacheFactory>(info.This())->cacheFactory;
//  cacheFactory.setStatisticInterval(Nan::To<int>(info[1]).FromJust());
//  info.GetReturnValue().Set(info.This());
//}
// NAN_METHOD(CacheFactory::SetSubscriptionAckInterval) {
//  Nan::HandleScope scope;
//  int argsLength = info.Length();
//  if(argsLength != 1 && info[0]->IsNumber()){
//    Nan::ThrowError("You must pass an integer value.");
//    return;
//  }
//  auto cacheFactory =
//  Nan::ObjectWrap::Unwrap<CacheFactory>(info.This())->cacheFactory;
//  cacheFactory.setSubscriptionAckInterval(Nan::To<int>(info[1]).FromJust());
//  info.GetReturnValue().Set(info.This());
//}
// NAN_METHOD(CacheFactory::SetSubscriptionEnabled) {
//  Nan::HandleScope scope;
//  int argsLength = info.Length();
//  if(argsLength != 1 && info[0]->IsBoolean()){
//    Nan::ThrowError("You must pass an bool value.");
//    return;
//  }
//  auto cacheFactory =
//  Nan::ObjectWrap::Unwrap<CacheFactory>(info.This())->cacheFactory;
//  cacheFactory.setSubscriptionEnabled(Nan::To<bool>(info[1]).FromJust());
//  info.GetReturnValue().Set(info.This());
//}
// NAN_METHOD(CacheFactory::SetSubscriptionMessageTrackingTimeout) {
//  Nan::HandleScope scope;
//  int argsLength = info.Length();
//  if(argsLength != 1 && info[0]->IsNumber()){
//    Nan::ThrowError("You must pass an integer value.");
//    return;
//  }
//  auto cacheFactory =
//  Nan::ObjectWrap::Unwrap<CacheFactory>(info.This())->cacheFactory;
//  cacheFactory.setSubscriptionMessageTrackingTimeout(Nan::To<int>(info[1]).FromJust());
//  info.GetReturnValue().Set(info.This());
//}
// NAN_METHOD(CacheFactory::SetSubscriptionRedundancy) {
//  Nan::HandleScope scope;
//  int argsLength = info.Length();
//  if(argsLength != 1 && info[0]->IsNumber()){
//    Nan::ThrowError("You must pass an integer value.");
//    return;
//  }
//  auto cacheFactory =
//  Nan::ObjectWrap::Unwrap<CacheFactory>(info.This())->cacheFactory;
//  cacheFactory.setSubscriptionRedundancy(Nan::To<int>(info[1]).FromJust());
//  info.GetReturnValue().Set(info.This());
//}
// NAN_METHOD(CacheFactory::SetThreadLocalConnections) {
//  Nan::HandleScope scope;
//  int argsLength = info.Length();
//  if(argsLength != 1 && info[0]->IsBoolean()){
//    Nan::ThrowError("You must pass an bool value.");
//    return;
//  }
//  auto cacheFactory =
//  Nan::ObjectWrap::Unwrap<CacheFactory>(info.This())->cacheFactory;
//  cacheFactory.setThreadLocalConnections(Nan::To<bool>(info[1]).FromJust());
//  info.GetReturnValue().Set(info.This());
//}
// NAN_METHOD(CacheFactory::SetUpdateLocatorListInterval) {
//  Nan::HandleScope scope;
//  int argsLength = info.Length();
//  if(argsLength != 1 && info[0]->IsNumber()){
//    Nan::ThrowError("You must pass an long value.");
//    return;
//  }
//  auto cacheFactory =
//  Nan::ObjectWrap::Unwrap<CacheFactory>(info.This())->cacheFactory;
//  cacheFactory.setUpdateLocatorListInterval(Nan::To<int64_t>(info[1]).FromJust());
//  info.GetReturnValue().Set(info.This());
//}

NAN_METHOD(CacheFactory::Create) {
  Nan::HandleScope scope;
  auto cacheFactory = Nan::ObjectWrap::Unwrap<CacheFactory>(info.This());
  auto cacheFactoryPtr = cacheFactory->cacheFactory;

  // Force the PDX Read Serialized to true no matter what
  cacheFactoryPtr.setPdxReadSerialized(true);

  auto cache = Cache::NewInstance(
      std::make_shared<apache::geode::client::Cache>(cacheFactoryPtr.create()));

  if (cacheFactory->callback != NULL) {
    Local<Value> argv[1] = {cache};
    Nan::Call(*(cacheFactory->callback), 1, argv);
  }
  info.GetReturnValue().Set(cache);
}
}  // namespace node_gemfire
