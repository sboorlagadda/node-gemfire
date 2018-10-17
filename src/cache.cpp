#include "cache.hpp"

#include <nan.h>
#include <node.h>
#include <v8.h>

#include <sstream>
#include <string>

#include <geode/Cache.hpp>
#include <geode/CacheFactory.hpp>
#include <geode/Execution.hpp>
#include <geode/FunctionService.hpp>
#include <geode/Query.hpp>
#include <geode/QueryService.hpp>
#include <geode/Region.hpp>
#include <geode/RegionFactory.hpp>

#include "conversions.hpp"
#include "dependencies.hpp"
#include "exceptions.hpp"
#include "functions.hpp"
#include "gemfire_worker.hpp"
#include "region.hpp"
#include "region_shortcuts.hpp"

using namespace v8;

namespace node_gemfire {

static std::shared_ptr<apache::geode::client::Cache> closeThisCache = nullptr;

static void closeCacheAtExit(void* arg) {
  if (closeThisCache != nullptr && !closeThisCache->isClosed()) {
    closeThisCache->close();
  }
}
NAN_MODULE_INIT(Cache::Init) {
  Nan::HandleScope scope;

  Local<FunctionTemplate> constructorTemplate = Nan::New<FunctionTemplate>();
  constructorTemplate->SetClassName(Nan::New("Cache").ToLocalChecked());

  constructorTemplate->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(constructorTemplate, "close", Cache::Close);
  Nan::SetPrototypeMethod(constructorTemplate, "executeFunction",
                          Cache::ExecuteFunction);
  Nan::SetPrototypeMethod(constructorTemplate, "executeQuery",
                          Cache::ExecuteQuery);
  Nan::SetPrototypeMethod(constructorTemplate, "createRegion",
                          Cache::CreateRegion);
  Nan::SetPrototypeMethod(constructorTemplate, "getRegion", Cache::GetRegion);
  Nan::SetPrototypeMethod(constructorTemplate, "rootRegions",
                          Cache::RootRegions);
  Nan::SetPrototypeMethod(constructorTemplate, "inspect", Cache::Inspect);

  constructor().Reset(Nan::GetFunction(constructorTemplate).ToLocalChecked());

  Nan::Set(target, Nan::New("Cache").ToLocalChecked(),
           Nan::GetFunction(constructorTemplate).ToLocalChecked());
}

v8::Local<v8::Object> Cache::NewInstance(
    std::shared_ptr<apache::geode::client::Cache> cache) {
  Nan::EscapableHandleScope scope;
  const unsigned int argc = 0;
  Local<Value> argv[argc] = {};
  Local<v8::Function> cons = Nan::New(Cache::constructor());
  Local<Object> instance = Nan::NewInstance(cons, argc, argv).ToLocalChecked();
  auto cacheWrapper = new Cache(cache);
  cacheWrapper->Wrap(instance);
  return scope.Escape(instance);
}

NAN_METHOD(Cache::Close) {
  Nan::HandleScope scope;

  auto cache = Nan::ObjectWrap::Unwrap<Cache>(info.This());
  if (cache != nullptr) {
    cache->close();
  }
}

void Cache::close() {
  if (!cache->isClosed()) {
    cache->close();
  }
}

class ExecuteQueryWorker : public GemfireWorker {
 public:
  ExecuteQueryWorker(
      std::shared_ptr<apache::geode::client::Query> query,
      std::shared_ptr<apache::geode::client::CacheableVector> queryParams,
      Nan::Callback* callback)
      : GemfireWorker(callback), query(query), queryParams(queryParams) {}

  void ExecuteGemfireWork() { selectResults = query->execute(queryParams); }

  void HandleOKCallback() {
    Nan::HandleScope scope;

    static const int argc = 2;
    Local<Value> argv[2] = {Nan::Undefined(), v8Value(selectResults)};
    callback->Call(argc, argv);
  }

  std::shared_ptr<apache::geode::client::Query> query;
  std::shared_ptr<apache::geode::client::CacheableVector> queryParams;
  std::shared_ptr<apache::geode::client::SelectResults> selectResults;
};

NAN_METHOD(Cache::ExecuteQuery) {
  Nan::HandleScope scope;

  int argsLength = info.Length();

  if (argsLength == 0 || !info[0]->IsString()) {
    Nan::ThrowError(
        "You must pass a query string and callback to executeQuery().");
    return;
  }

  if (argsLength < 2) {
    Nan::ThrowError("You must pass a callback to executeQuery().");
    return;
  }

  Local<Function> callbackFunction;
  Local<Value> poolNameValue(Nan::Undefined());
  Local<Value> queryParams;

  if (info[1]->IsFunction()) {
    callbackFunction = info[1].As<Function>();
  } else if (argsLength > 2 && info[2]->IsFunction()) {
    callbackFunction = info[2].As<Function>();

    if (info[1]->IsObject() && !info[1]->IsFunction()) {
      Local<Object> optionsObject = info[1]->ToObject();
      poolNameValue = optionsObject->Get(Nan::New("poolName").ToLocalChecked());
    }
  } else if (argsLength > 3 && info[3]->IsFunction()) {
    callbackFunction = info[3].As<Function>();

    if (info[1]->IsArray() && !info[1]->IsFunction()) {
      queryParams = info[1];
    }

    if (info[2]->IsObject() && !info[2]->IsFunction()) {
      Local<Object> optionsObject = info[2]->ToObject();
      poolNameValue = optionsObject->Get(Nan::New("poolName").ToLocalChecked());
    }
  } else {
    Nan::ThrowError(
        "You must pass a function as the callback to executeQuery().");
    return;
  }

  auto cacheWrapper = Nan::ObjectWrap::Unwrap<Cache>(info.This());
  auto cache = cacheWrapper->cache;

  if (cache->isClosed()) {
    Nan::ThrowError("Cannot execute query; cache is closed.");
    return;
  }

  std::shared_ptr<apache::geode::client::QueryService> queryService;
  std::shared_ptr<apache::geode::client::CacheableVector> geodeQueryParams =
      nullptr;

  std::string queryString(*Nan::Utf8String(info[0]));

  try {
    if (poolNameValue->IsUndefined()) {
      queryService = cache->getQueryService();
    } else {
      std::string poolName(*Nan::Utf8String(poolNameValue));
      auto pool = getPool(poolNameValue);

      if (pool == nullptr) {
        auto poolName = *Nan::Utf8String(poolNameValue);
        std::stringstream errorMessageStream;
        errorMessageStream << "executeQuery: `" << poolName
                           << "` is not a valid pool name";
        Nan::ThrowError(errorMessageStream.str().c_str());
        return;
      }

      queryService = cache->getQueryService(poolName.c_str());
    }
  } catch (const apache::geode::client::Exception& exception) {
    ThrowGemfireException(exception);
    return;
  }
  if (!(queryParams.IsEmpty() || queryParams->IsUndefined())) {
    geodeQueryParams = gemfireVector(queryParams.As<Array>(), *cache);
  }

  auto query = queryService->newQuery(queryString);

  auto callback = new Nan::Callback(callbackFunction);

  auto worker = new ExecuteQueryWorker(query, geodeQueryParams, callback);
  Nan::AsyncQueueWorker(worker);

  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(Cache::CreateRegion) {
  Nan::HandleScope scope;

  if (info.Length() < 1) {
    Nan::ThrowError(
        "createRegion: You must pass the name of a GemFire region to create "
        "and a region configuration object.");
    return;
  }

  if (!info[0]->IsString()) {
    Nan::ThrowError(
        "createRegion: You must pass a string as the name of a GemFire "
        "region.");
    return;
  }

  if (!info[1]->IsObject()) {
    Nan::ThrowError(
        "createRegion: You must pass a configuration object as the second "
        "argument.");
    return;
  }

  Local<Object> regionConfiguration(info[1]->ToObject());

  Local<Value> regionType(
      regionConfiguration->Get(Nan::New("type").ToLocalChecked()));
  if (regionType->IsUndefined()) {
    Nan::ThrowError(
        "createRegion: The region configuration object must have a type "
        "property.");
    return;
  }

  Local<Value> regionPoolName(
      regionConfiguration->Get(Nan::New("poolName").ToLocalChecked()));

  auto regionShortcut = getRegionShortcut(*Nan::Utf8String(regionType));
  if (regionShortcut == invalidRegionShortcut) {
    Nan::ThrowError(
        "createRegion: This type is not a valid GemFire client region type");
    return;
  }

  auto cacheWrapper = Nan::ObjectWrap::Unwrap<Cache>(info.This());
  std::shared_ptr<apache::geode::client::Cache> cache(cacheWrapper->cache);

  std::shared_ptr<apache::geode::client::Region> region;
  try {
    auto regionFactory = cache->createRegionFactory(regionShortcut);

    if (!regionPoolName->IsUndefined()) {
      regionFactory.setPoolName(*Nan::Utf8String(regionPoolName));
    }

    region = regionFactory.create(*Nan::Utf8String(info[0]));
  } catch (const apache::geode::client::Exception& exception) {
    ThrowGemfireException(exception);
    return;
  }
  info.GetReturnValue().Set(Region::NewInstance(region));
}

NAN_METHOD(Cache::GetRegion) {
  Nan::HandleScope scope;

  if (info.Length() != 1) {
    Nan::ThrowError("You must pass the name of a GemFire region to getRegion.");
    info.GetReturnValue().Set(Nan::Undefined());
    return;
  }

  if (!info[0]->IsString()) {
    Nan::ThrowError(
        "You must pass a string as the name of a GemFire region to getRegion.");
    info.GetReturnValue().Set(Nan::Undefined());
    return;
  }

  auto cache = Nan::ObjectWrap::Unwrap<Cache>(info.This())->cache;
  auto region = cache->getRegion(*Nan::Utf8String(info[0]));

  if (region == nullptr) {
    info.GetReturnValue().Set(Nan::Undefined());
  } else {
    info.GetReturnValue().Set(Region::NewInstance(region));
  }
}

NAN_METHOD(Cache::RootRegions) {
  Nan::HandleScope scope;

  auto cache = Nan::ObjectWrap::Unwrap<Cache>(info.This())->cache;
  auto regions = cache->rootRegions();
  auto size = regions.size();
  auto rootRegions = Nan::New<Array>(size);

  for (decltype(size) i = 0; i < size; i++) {
    rootRegions->Set(i, Region::NewInstance(regions[i]));
  }

  info.GetReturnValue().Set(rootRegions);
}

NAN_METHOD(Cache::Inspect) {
  Nan::HandleScope scope;
  info.GetReturnValue().Set(Nan::New("[Cache]").ToLocalChecked());
}

NAN_METHOD(Cache::ExecuteFunction) {
  Nan::HandleScope scope;

  auto cache = Nan::ObjectWrap::Unwrap<Cache>(info.This())->cache;
  if (cache->isClosed()) {
    Nan::ThrowError("Cannot execute function; cache is closed.");
    return;
  }

  Local<Value> poolNameValue = Nan::Undefined();
  if (info[1]->IsObject() && !info[1]->IsArray()) {
    auto optionsObject = info[1]->ToObject();
    auto filter = optionsObject->Get(Nan::New("filter").ToLocalChecked());
    if (!filter->IsUndefined()) {
      Nan::ThrowError(
          "You cannot pass a filter to executeFunction for a Cache.");
      return;
    }

    poolNameValue = optionsObject->Get(Nan::New("poolName").ToLocalChecked());
  }

  try {
    auto pool = getPool(poolNameValue);

    if (pool == nullptr) {
      auto poolName = *Nan::Utf8String(poolNameValue);
      std::stringstream errorMessageStream;
      errorMessageStream << "executeFunction: `" << poolName
                         << "` is not a valid pool name";
      Nan::ThrowError(errorMessageStream.str().c_str());
      return;
    }

    auto execution = apache::geode::client::FunctionService::onServers(pool);
    info.GetReturnValue().Set(executeFunction(info, *cache, execution));
  } catch (const apache::geode::client::Exception& exception) {
    ThrowGemfireException(exception);
  }
}

}  // namespace node_gemfire
