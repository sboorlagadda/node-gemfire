#include "region.hpp"

#include <uv.h>

#include <sstream>
#include <string>
#include <vector>

#include <geode/FunctionService.hpp>
#include <geode/Region.hpp>
#include <geode/internal/chrono/duration.hpp>

#include "cache.hpp"
#include "conversions.hpp"
#include "dependencies.hpp"
#include "events.hpp"
#include "exceptions.hpp"
#include "functions.hpp"
#include "gemfire_worker.hpp"
#include "region_event_registry.hpp"

using namespace v8;

namespace node_gemfire {

inline bool isFunctionOrUndefined(const Local<Value>& value) {
  return value->IsUndefined() || value->IsFunction();
}

inline Nan::Callback* getCallback(const Local<Value>& value) {
  if (value->IsUndefined()) {
    return NULL;
  }
  return new Nan::Callback(Local<Function>::Cast(value));
}

v8::Local<v8::Object> Region::NewInstance(
    std::shared_ptr<apache::geode::client::Region> regionPtr) {
  Nan::EscapableHandleScope scope;
  const unsigned int argc = 0;
  Local<Value> argv[argc] = {};
  Local<Object> instance(
      Nan::New(Region::constructor())->NewInstance(argc, argv));
  Region* region = new Region(regionPtr);
  RegionEventRegistry::getInstance()->add(region);
  region->Wrap(instance);
  return scope.Escape(instance);
}

class GemfireEventedWorker : public GemfireWorker {
 public:
  GemfireEventedWorker(const Local<Object>& v8Object, Nan::Callback* callback)
      : GemfireWorker(callback) {
    SaveToPersistent("v8Object", v8Object);
  }

  virtual void HandleOKCallback() {
    if (callback) {
      Nan::Call(*callback, 0, NULL);
    }
  }

  virtual void HandleErrorCallback() {
    Nan::HandleScope scope;
    if (callback) {
      Local<Value> argv[1] = {errorObject()};
      Nan::Call(*callback, 1, argv);
    } else {
      Local<Object> v8Object = GetFromPersistent("v8Object")->ToObject();
      emitError(v8Object, errorObject());
    }
  }
};

class ClearWorker : public GemfireEventedWorker {
 public:
  ClearWorker(const Local<Object>& regionObject, Region* region,
              Nan::Callback* callback)
      : GemfireEventedWorker(regionObject, callback), region(region) {}

  void ExecuteGemfireWork() {
    // Workaround: We don't want to call clear on the region if the cache is
    // closed. After cache is cleared, getCache() will throw an exception,
    // whereas clear() causes a segfault.
    region->region->getRegionService().isClosed();
    region->region->clear();
  }
  Region* region;
};

NAN_METHOD(Region::Clear) {
  Nan::HandleScope scope;

  if (!isFunctionOrUndefined(info[0])) {
    Nan::ThrowError("You must pass a function as the callback to clear().");
    return;
  }
  auto region = Nan::ObjectWrap::Unwrap<Region>(info.Holder());
  auto callback = getCallback(info[0]);
  ClearWorker* worker = new ClearWorker(info.Holder(), region, callback);
  Nan::AsyncQueueWorker(worker);

  info.GetReturnValue().Set(info.Holder());
}

std::string unableToPutValueError(Local<Value> v8Value) {
  std::stringstream errorMessageStream;
  errorMessageStream << "Unable to put value "
                     << *String::Utf8Value(v8Value->ToDetailString());
  return errorMessageStream.str();
}

apache::geode::client::Cache& getCacheFromRegion(
    std::shared_ptr<apache::geode::client::Region> regionPtr) {
  Nan::HandleScope scope;
  try {
    return regionPtr->getCache();
  } catch (const apache::geode::client::RegionDestroyedException& exception) {
    ThrowGemfireException(exception);
  }
}

class PutWorker : public GemfireEventedWorker {
 public:
  PutWorker(const Local<Object>& regionObject, Region* region,
            const std::shared_ptr<apache::geode::client::CacheableKey>& keyPtr,
            const std::shared_ptr<apache::geode::client::Cacheable>& valuePtr,
            Nan::Callback* callback)
      : GemfireEventedWorker(regionObject, callback),
        region(region),
        keyPtr(keyPtr),
        valuePtr(valuePtr) {}

  void ExecuteGemfireWork() {
    if (keyPtr == nullptr) {
      SetError("InvalidKeyError", "Invalid GemFire key.");
      return;
    }
    if (valuePtr == nullptr) {
      SetError("InvalidValueError", "Invalid GemFire value.");
      return;
    }
    region->region->put(keyPtr, valuePtr);
  }
  Region* region;
  std::shared_ptr<apache::geode::client::CacheableKey> keyPtr;
  std::shared_ptr<apache::geode::client::Cacheable> valuePtr;
};

NAN_METHOD(Region::Put) {
  Nan::HandleScope scope;
  unsigned int argsLength = info.Length();

  if (argsLength < 2) {
    Nan::ThrowError("You must pass a key and value to put().");
    return;
  }
  if (!isFunctionOrUndefined(info[2])) {
    Nan::ThrowError("You must pass a function as the callback to put().");
    return;
  }

  auto region = Nan::ObjectWrap::Unwrap<Region>(info.Holder());

  auto& cache = region->region->getCache();

  std::shared_ptr<apache::geode::client::CacheableKey> keyPtr(
      gemfireKey(info[0], cache));
  std::shared_ptr<apache::geode::client::Cacheable> valuePtr(
      gemfireValue(info[1], cache));

  auto callback = getCallback(info[2]);
  PutWorker* putWorker =
      new PutWorker(info.Holder(), region, keyPtr, valuePtr, callback);
  Nan::AsyncQueueWorker(putWorker);

  info.GetReturnValue().Set(info.Holder());
}

NAN_METHOD(Region::PutSync) {
  Nan::HandleScope scope;

  try {
    unsigned int argsLength = info.Length();
    if (argsLength != 2) {
      Nan::ThrowError("You must pass a key and value to putSync().");
      info.GetReturnValue().Set(Nan::Undefined());
      return;
    }

    auto region = Nan::ObjectWrap::Unwrap<Region>(info.Holder());

    auto& cachePtr = region->region->getCache();

    auto keyPtr = gemfireKey(info[0], cachePtr);
    if (keyPtr == nullptr) {
      Nan::ThrowError("Invalid GemFire key.");
      info.GetReturnValue().Set(Nan::Undefined());
      return;
    }

    auto valuePtr = gemfireValue(info[1], cachePtr);
    if (valuePtr == nullptr) {
      Nan::ThrowError("Invalid GemFire value.");
      info.GetReturnValue().Set(Nan::Undefined());
      return;
    }

    region->region->put(keyPtr, valuePtr);
    info.GetReturnValue().Set(info.Holder());
  } catch (apache::geode::client::Exception& exception) {
    ThrowGemfireException(exception);
    info.GetReturnValue().Set(Nan::Undefined());
  }
}

class GetWorker : public GemfireWorker {
 public:
  GetWorker(Nan::Callback* callback,
            const std::shared_ptr<apache::geode::client::Region>& regionPtr,
            const std::shared_ptr<apache::geode::client::CacheableKey>& keyPtr)
      : GemfireWorker(callback), regionPtr(regionPtr), keyPtr(keyPtr) {}

  void ExecuteGemfireWork() {
    if (keyPtr == nullptr) {
      SetError("InvalidKeyError", "Invalid GemFire key.");
      return;
    }

    valuePtr = regionPtr->get(keyPtr);

    // TODO switching up behavior no error for key not found
    /*
    if (valuePtr == nullptr) {
      SetError("KeyNotFoundError", "Key not found in region.");
    }
    */
  }

  void HandleOKCallback() {
    Nan::HandleScope scope;
    Local<Value> argv[2] = {Nan::Undefined(), v8Value(valuePtr)};
    Nan::Call(*callback, 2, argv);
  }

  std::shared_ptr<apache::geode::client::Region> regionPtr;
  std::shared_ptr<apache::geode::client::CacheableKey> keyPtr;
  std::shared_ptr<apache::geode::client::Cacheable> valuePtr;
};

NAN_METHOD(Region::Get) {
  Nan::HandleScope scope;

  unsigned int argsLength = info.Length();

  if (argsLength != 2) {
    Nan::ThrowError("You must pass a key and a callback to get().");
    return;
  }

  if (!info[1]->IsFunction()) {
    Nan::ThrowError("You must pass a function as the callback to get().");
    return;
  }

  auto region = Nan::ObjectWrap::Unwrap<Region>(info.Holder());
  auto regionPtr = region->region;
  auto& cachePtr = regionPtr->getCache();

  auto keyPtr = gemfireKey(info[0], cachePtr);

  auto callback = new Nan::Callback(info[1].As<Function>());
  auto getWorker = new GetWorker(callback, regionPtr, keyPtr);
  Nan::AsyncQueueWorker(getWorker);

  info.GetReturnValue().Set(info.Holder());
}

NAN_METHOD(Region::GetSync) {
  Nan::HandleScope scope;
  std::shared_ptr<apache::geode::client::Cacheable> valuePtr = nullptr;
  try {
    unsigned int argsLength = info.Length();
    if (argsLength == 0) {
      Nan::ThrowError("You must pass a key to getSync().");
      info.GetReturnValue().Set(Nan::Undefined());
      return;
    }

    auto region = Nan::ObjectWrap::Unwrap<Region>(info.Holder());
    auto regionPtr = region->region;

    auto& cachePtr = regionPtr->getCache();
    auto keyPtr = gemfireKey(info[0], cachePtr);
    valuePtr = regionPtr->get(keyPtr);
  } catch (apache::geode::client::Exception& exception) {
    ThrowGemfireException(exception);
  }
  info.GetReturnValue().Set(v8Value(valuePtr));
}

class GetAllWorker : public GemfireWorker {
 public:
  GetAllWorker(
      const std::shared_ptr<apache::geode::client::Region>& regionPtr,
      const std::vector<std::shared_ptr<apache::geode::client::CacheableKey>>&
          gemfireKeysPtr,
      Nan::Callback* callback)
      : GemfireWorker(callback),
        regionPtr(regionPtr),
        gemfireKeysPtr(gemfireKeysPtr) {}

  void ExecuteGemfireWork() {
    if (gemfireKeysPtr.empty()) {
      return;
    }

    resultsPtr = regionPtr->getAll(gemfireKeysPtr);
  }

  void HandleOKCallback() {
    Nan::HandleScope scope;

    Local<Value> argv[2] = {Nan::Undefined(), v8Value(resultsPtr)};
    Nan::Call(*callback, 2, argv);
  }

 private:
  std::shared_ptr<apache::geode::client::Region> regionPtr;
  std::vector<std::shared_ptr<apache::geode::client::CacheableKey>>
      gemfireKeysPtr;
  apache::geode::client::HashMapOfCacheable resultsPtr;
};

NAN_METHOD(Region::GetAll) {
  Nan::HandleScope scope;

  if (info.Length() == 0 || !info[0]->IsArray()) {
    Nan::ThrowError(
        "You must pass an array of keys and a callback to getAll().");
    return;
  }

  if (info.Length() == 1) {
    Nan::ThrowError("You must pass a callback to getAll().");
    return;
  }

  if (!info[1]->IsFunction()) {
    Nan::ThrowError("You must pass a function as the callback to getAll().");
    return;
  }

  auto region = Nan::ObjectWrap::Unwrap<Region>(info.Holder());
  auto regionPtr = region->region;

  auto& cachePtr = regionPtr->getCache();

  auto gemfireKeysPtr = gemfireKeys(Local<Array>::Cast(info[0]), cachePtr);

  auto callback = new Nan::Callback(info[1].As<Function>());

  auto worker = new GetAllWorker(regionPtr, gemfireKeysPtr, callback);
  Nan::AsyncQueueWorker(worker);

  info.GetReturnValue().Set(info.Holder());
}

NAN_METHOD(Region::GetAllSync) {
  Nan::HandleScope scope;
  try {
    if (info.Length() != 1 || !info[0]->IsArray()) {
      Nan::ThrowError("You must pass an array of keys to getAllSync().");
      info.GetReturnValue().Set(Nan::Undefined());
      return;
    }
    auto region = Nan::ObjectWrap::Unwrap<Region>(info.Holder());
    auto regionPtr = region->region;
    auto& cachePtr = regionPtr->getCache();

    auto gemfireKeysPtr = gemfireKeys(Local<Array>::Cast(info[0]), cachePtr);
    if (gemfireKeysPtr.empty()) {
      info.GetReturnValue().Set(
          v8Object(apache::geode::client::HashMapOfCacheable{}));
    } else {
      auto resultsPtr = regionPtr->getAll(gemfireKeysPtr);
      info.GetReturnValue().Set(v8Value(resultsPtr));
    }
  } catch (apache::geode::client::Exception& exception) {
    ThrowGemfireException(exception);
    info.GetReturnValue().Set(Nan::Undefined());
  }
}

class PutAllWorker : public GemfireEventedWorker {
 public:
  PutAllWorker(const Local<Object>& regionObject,
               const std::shared_ptr<apache::geode::client::Region>& regionPtr,
               apache::geode::client::HashMapOfCacheable hashMapPtr,
               Nan::Callback* callback)
      : GemfireEventedWorker(regionObject, callback),
        regionPtr(regionPtr),
        hashMapPtr(std::move(hashMapPtr)) {}

  void ExecuteGemfireWork() { regionPtr->putAll(hashMapPtr); }

 private:
  std::shared_ptr<apache::geode::client::Region> regionPtr;
  apache::geode::client::HashMapOfCacheable hashMapPtr;
};

NAN_METHOD(Region::PutAll) {
  Nan::HandleScope scope;
  if (info.Length() == 0 || !info[0]->IsObject()) {
    Nan::ThrowError("You must pass an object and a callback to putAll().");
    return;
  }
  if (!isFunctionOrUndefined(info[1])) {
    Nan::ThrowError("You must pass a function as the callback to putAll().");
    return;
  }
  auto region = Nan::ObjectWrap::Unwrap<Region>(info.Holder());
  auto regionPtr = region->region;
  auto& cachePtr = regionPtr->getCache();

  auto hashMapPtr = gemfireHashMap(info[0]->ToObject(), cachePtr);
  auto callback = getCallback(info[1]);
  auto worker =
      new PutAllWorker(info.Holder(), regionPtr, hashMapPtr, callback);
  Nan::AsyncQueueWorker(worker);

  info.GetReturnValue().Set(info.Holder());
}

NAN_METHOD(Region::PutAllSync) {
  Nan::HandleScope scope;
  try {
    if (info.Length() != 1 || !info[0]->IsObject()) {
      Nan::ThrowError("You must pass an object to putAllSync().");
      info.GetReturnValue().Set(Nan::Undefined());
      return;
    }

    auto region = Nan::ObjectWrap::Unwrap<Region>(info.Holder());
    auto regionPtr = region->region;
    auto& cachePtr = regionPtr->getCache();

    auto hashMapPtr = gemfireHashMap(info[0]->ToObject(), cachePtr);
    regionPtr->putAll(hashMapPtr);
    info.GetReturnValue().Set(info.Holder());
  } catch (const apache::geode::client::Exception& exception) {
    ThrowGemfireException(exception);
    info.GetReturnValue().Set(Nan::Undefined());
  }
}

class RemoveWorker : public GemfireEventedWorker {
 public:
  RemoveWorker(
      const Local<Object>& regionObject,
      const std::shared_ptr<apache::geode::client::Region>& regionPtr,
      const std::shared_ptr<apache::geode::client::CacheableKey>& keyPtr,
      Nan::Callback* callback)
      : GemfireEventedWorker(regionObject, callback),
        regionPtr(regionPtr),
        keyPtr(keyPtr) {}

  void ExecuteGemfireWork() {
    if (keyPtr == nullptr) {
      SetError("InvalidKeyError", "Invalid GemFire key.");
      return;
    }

    try {
      regionPtr->destroy(keyPtr);
    } catch (const apache::geode::client::EntryNotFoundException& exception) {
      SetError("KeyNotFoundError", "Key not found in region.");
    }
  }

  std::shared_ptr<apache::geode::client::Region> regionPtr;
  std::shared_ptr<apache::geode::client::CacheableKey> keyPtr;
};

NAN_METHOD(Region::Remove) {
  Nan::HandleScope scope;

  if (info.Length() < 1) {
    Nan::ThrowError("You must pass a key to remove().");
    return;
  }

  if (!isFunctionOrUndefined(info[1])) {
    Nan::ThrowError("You must pass a function as the callback to remove().");
    return;
  }

  auto region = Nan::ObjectWrap::Unwrap<Region>(info.Holder());
  auto regionPtr = region->region;
  auto& cachePtr = regionPtr->getCache();

  auto keyPtr = gemfireKey(info[0], cachePtr);
  auto callback = getCallback(info[1]);
  auto worker = new RemoveWorker(info.Holder(), regionPtr, keyPtr, callback);
  Nan::AsyncQueueWorker(worker);

  info.GetReturnValue().Set(info.Holder());
}

NAN_METHOD(Region::ExecuteFunction) {
  Nan::HandleScope scope;

  auto region = Nan::ObjectWrap::Unwrap<Region>(info.Holder());
  auto regionPtr = region->region;
  auto& cachePtr = regionPtr->getCache();

  try {
    auto executionPtr =
        apache::geode::client::FunctionService::onRegion(regionPtr);
    info.GetReturnValue().Set(executeFunction(info, cachePtr, executionPtr));
  } catch (const apache::geode::client::Exception& exception) {
    ThrowGemfireException(exception);
    return;
  }
}

NAN_METHOD(Region::Inspect) {
  Nan::HandleScope scope;

  auto region = Nan::ObjectWrap::Unwrap<Region>(info.Holder());
  auto regionPtr = region->region;

  auto name = regionPtr->getName();

  std::stringstream inspectStream;
  inspectStream << "[Region name=\"" << name << "\"]";
  info.GetReturnValue().Set(
      Nan::New(inspectStream.str().c_str()).ToLocalChecked());
}

NAN_GETTER(Region::Name) {
  Nan::HandleScope scope;

  auto region = Nan::ObjectWrap::Unwrap<Region>(info.Holder());
  auto regionPtr = region->region;

  info.GetReturnValue().Set(Nan::New(regionPtr->getName()).ToLocalChecked());
}

NAN_GETTER(Region::Attributes) {
  Nan::HandleScope scope;

  auto region = Nan::ObjectWrap::Unwrap<Region>(info.Holder());
  auto regionPtr = region->region;

  auto& regionAttributesPtr = regionPtr->getAttributes();

  Local<Object> returnValue(Nan::New<Object>());

  Nan::DefineOwnProperty(returnValue,
                         Nan::New("cachingEnabled").ToLocalChecked(),
                         Nan::New(regionAttributesPtr.getCachingEnabled()),
                         static_cast<PropertyAttribute>(ReadOnly | DontDelete));

  Nan::DefineOwnProperty(
      returnValue, Nan::New("clientNotificationEnabled").ToLocalChecked(),
      Nan::New(regionAttributesPtr.getClientNotificationEnabled()),
      static_cast<PropertyAttribute>(ReadOnly | DontDelete));

  Nan::DefineOwnProperty(
      returnValue, Nan::New("concurrencyChecksEnabled").ToLocalChecked(),
      Nan::New(regionAttributesPtr.getConcurrencyChecksEnabled()),
      static_cast<PropertyAttribute>(ReadOnly | DontDelete));

  Nan::DefineOwnProperty(returnValue,
                         Nan::New("concurrencyLevel").ToLocalChecked(),
                         Nan::New(regionAttributesPtr.getConcurrencyLevel()),
                         static_cast<PropertyAttribute>(ReadOnly | DontDelete));

  Nan::DefineOwnProperty(
      returnValue, Nan::New("diskPolicy").ToLocalChecked(),
      Nan::New(static_cast<int32_t>(regionAttributesPtr.getDiskPolicy())),
      static_cast<PropertyAttribute>(ReadOnly | DontDelete));

  Nan::DefineOwnProperty(
      returnValue, Nan::New("entryIdleTimeout").ToLocalChecked(),
      Nan::New(apache::geode::internal::chrono::duration::to_string(
                   regionAttributesPtr.getEntryIdleTimeout()))
          .ToLocalChecked(),
      static_cast<PropertyAttribute>(ReadOnly | DontDelete));

  Nan::DefineOwnProperty(
      returnValue, Nan::New("entryTimeToLive").ToLocalChecked(),
      Nan::New(apache::geode::internal::chrono::duration::to_string(
                   regionAttributesPtr.getEntryTimeToLive()))
          .ToLocalChecked(),
      static_cast<PropertyAttribute>(ReadOnly | DontDelete));

  Nan::DefineOwnProperty(returnValue,
                         Nan::New("initialCapacity").ToLocalChecked(),
                         Nan::New(regionAttributesPtr.getInitialCapacity()),
                         static_cast<PropertyAttribute>(ReadOnly | DontDelete));

  Nan::DefineOwnProperty(returnValue, Nan::New("loadFactor").ToLocalChecked(),
                         Nan::New(regionAttributesPtr.getLoadFactor()),
                         static_cast<PropertyAttribute>(ReadOnly | DontDelete));

  Nan::DefineOwnProperty(returnValue,
                         Nan::New("lruEntriesLimit").ToLocalChecked(),
                         Nan::New(regionAttributesPtr.getLruEntriesLimit()),
                         static_cast<PropertyAttribute>(ReadOnly | DontDelete));

  Nan::DefineOwnProperty(
      returnValue, Nan::New("lruEvicationAction").ToLocalChecked(),
      Nan::New(
          static_cast<int32_t>(regionAttributesPtr.getLruEvictionAction())),
      static_cast<PropertyAttribute>(ReadOnly | DontDelete));

  Nan::DefineOwnProperty(
      returnValue, Nan::New("poolName").ToLocalChecked(),
      Nan::New(regionAttributesPtr.getPoolName()).ToLocalChecked(),
      static_cast<PropertyAttribute>(ReadOnly | DontDelete));

  Nan::DefineOwnProperty(
      returnValue, Nan::New("regionIdleTimeout").ToLocalChecked(),
      Nan::New(apache::geode::internal::chrono::duration::to_string(
                   regionAttributesPtr.getRegionIdleTimeout()))
          .ToLocalChecked(),
      static_cast<PropertyAttribute>(ReadOnly | DontDelete));

  Nan::DefineOwnProperty(
      returnValue, Nan::New("regionTimeToLive").ToLocalChecked(),
      Nan::New(apache::geode::internal::chrono::duration::to_string(
                   regionAttributesPtr.getRegionTimeToLive()))
          .ToLocalChecked(),
      static_cast<PropertyAttribute>(ReadOnly | DontDelete));

  info.GetReturnValue().Set(returnValue);
}

template <typename T>
class AbstractQueryWorker : public GemfireWorker {
 public:
  AbstractQueryWorker(
      const std::shared_ptr<apache::geode::client::Region>& regionPtr,
      const std::string& queryPredicate, Nan::Callback* callback)
      : GemfireWorker(callback),
        regionPtr(regionPtr),
        queryPredicate(queryPredicate) {}

  void HandleOKCallback() {
    Local<Value> argv[2] = {Nan::Undefined(), v8Value(resultPtr)};
    Nan::Call(*callback, 2, argv);
  }

  std::shared_ptr<apache::geode::client::Region> regionPtr;
  std::string queryPredicate;
  T resultPtr;
};

class QueryWorker : public AbstractQueryWorker<
                        std::shared_ptr<apache::geode::client::SelectResults>> {
 public:
  QueryWorker(const std::shared_ptr<apache::geode::client::Region>& regionPtr,
              const std::string& queryPredicate, Nan::Callback* callback)
      : AbstractQueryWorker<
            std::shared_ptr<apache::geode::client::SelectResults>>(
            regionPtr, queryPredicate, callback) {}

  void ExecuteGemfireWork() { resultPtr = regionPtr->query(queryPredicate); }

  static std::string name() { return "query()"; }
};

class SelectValueWorker
    : public AbstractQueryWorker<
          std::shared_ptr<apache::geode::client::Cacheable>> {
 public:
  SelectValueWorker(
      const std::shared_ptr<apache::geode::client::Region>& regionPtr,
      const std::string& queryPredicate, Nan::Callback* callback)
      : AbstractQueryWorker<std::shared_ptr<apache::geode::client::Cacheable>>(
            regionPtr, queryPredicate, callback) {}

  void ExecuteGemfireWork() {
    resultPtr = regionPtr->selectValue(queryPredicate.c_str());
  }

  static std::string name() { return "selectValue()"; }
};

class ExistsValueWorker : public AbstractQueryWorker<bool> {
 public:
  ExistsValueWorker(
      const std::shared_ptr<apache::geode::client::Region>& regionPtr,
      const std::string& queryPredicate, Nan::Callback* callback)
      : AbstractQueryWorker<bool>(regionPtr, queryPredicate, callback) {}

  void ExecuteGemfireWork() {
    resultPtr = regionPtr->existsValue(queryPredicate.c_str());
  }

  static std::string name() { return "existsValue()"; }
};

template <typename T>
NAN_METHOD(Region::Query) {
  Nan::HandleScope scope;

  if (info.Length() < 2) {
    std::stringstream errorStream;
    errorStream << "You must pass a query predicate string and a callback to "
                << T::name() << ".";
    Nan::ThrowError(errorStream.str().c_str());
    return;
  }

  if (!info[1]->IsFunction()) {
    std::stringstream errorStream;
    errorStream << "You must pass a function as the callback to " << T::name()
                << ".";
    Nan::ThrowError(errorStream.str().c_str());
    return;
  }

  auto region = Nan::ObjectWrap::Unwrap<Region>(info.Holder());

  auto queryPredicate = *Nan::Utf8String(info[0]);
  auto callback = new Nan::Callback(info[1].As<Function>());

  auto worker = new T(region->region, queryPredicate, callback);
  Nan::AsyncQueueWorker(worker);

  info.GetReturnValue().Set(info.Holder());
}

class ServerKeysWorker : public GemfireWorker {
 public:
  ServerKeysWorker(
      const std::shared_ptr<apache::geode::client::Region>& regionPtr,
      Nan::Callback* callback)
      : GemfireWorker(callback), regionPtr(regionPtr) {}

  void ExecuteGemfireWork() { keysVectorPtr = regionPtr->serverKeys(); }

  void HandleOKCallback() {
    Local<Value> argv[2] = {Nan::Undefined(), v8Value(keysVectorPtr)};
    Nan::Call(*callback, 2, argv);
  }

 private:
  std::shared_ptr<apache::geode::client::Region> regionPtr;
  std::vector<std::shared_ptr<apache::geode::client::CacheableKey>>
      keysVectorPtr;
};

NAN_METHOD(Region::ServerKeys) {
  Nan::HandleScope scope;

  if (info.Length() == 0) {
    Nan::ThrowError("You must pass a callback to serverKeys().");
    return;
  }

  if (!info[0]->IsFunction()) {
    Nan::ThrowError(
        "You must pass a function as the callback to serverKeys().");
    return;
  }

  auto region = Nan::ObjectWrap::Unwrap<Region>(info.Holder());
  auto callback = new Nan::Callback(info[0].As<Function>());

  auto worker = new ServerKeysWorker(region->region, callback);
  Nan::AsyncQueueWorker(worker);
}

class KeysWorker : public GemfireWorker {
 public:
  KeysWorker(const std::shared_ptr<apache::geode::client::Region>& regionPtr,
             Nan::Callback* callback)
      : GemfireWorker(callback), regionPtr(regionPtr) {}

  void ExecuteGemfireWork() { regionPtr->keys(); }

  void HandleOKCallback() {
    Local<Value> argv[2] = {Nan::Undefined(), v8Value(keysVectorPtr)};
    Nan::Call(*callback, 2, argv);
  }

 private:
  std::shared_ptr<apache::geode::client::Region> regionPtr;
  std::vector<std::shared_ptr<apache::geode::client::CacheableKey>>
      keysVectorPtr;
};

NAN_METHOD(Region::Keys) {
  Nan::HandleScope scope;

  if (info.Length() == 0) {
    Nan::ThrowError("You must pass a callback to keys().");
    return;
  }

  if (!info[0]->IsFunction()) {
    Nan::ThrowError("You must pass a function as the callback to keys().");
    return;
  }

  auto region = Nan::ObjectWrap::Unwrap<Region>(info.Holder());
  auto callback = new Nan::Callback(info[0].As<Function>());

  auto worker = new KeysWorker(region->region, callback);
  Nan::AsyncQueueWorker(worker);
}

NAN_METHOD(Region::RegisterAllKeys) {
  Nan::HandleScope scope;

  auto region = Nan::ObjectWrap::Unwrap<Region>(info.Holder());
  try {
    region->region->registerAllKeys();
  } catch (const apache::geode::client::Exception& exception) {
    ThrowGemfireException(exception);
  }
}

NAN_METHOD(Region::UnregisterAllKeys) {
  Nan::HandleScope scope;

  auto region = Nan::ObjectWrap::Unwrap<Region>(info.Holder());
  try {
    region->region->unregisterAllKeys();
  } catch (const apache::geode::client::Exception& exception) {
    ThrowGemfireException(exception);
  }
}

class ValuesWorker : public GemfireWorker {
 public:
  ValuesWorker(const std::shared_ptr<apache::geode::client::Region>& regionPtr,
               Nan::Callback* callback)
      : GemfireWorker(callback), regionPtr(regionPtr) {}

  void ExecuteGemfireWork() { valuesVectorPtr = regionPtr->values(); }

  void HandleOKCallback() {
    Local<Value> argv[2] = {Nan::Undefined(), v8Value(valuesVectorPtr)};
    Nan::Call(*callback, 2, argv);
  }

 private:
  std::shared_ptr<apache::geode::client::Region> regionPtr;
  std::vector<std::shared_ptr<apache::geode::client::Cacheable>>
      valuesVectorPtr;
};

NAN_METHOD(Region::Values) {
  Nan::HandleScope scope;

  if (info.Length() == 0) {
    Nan::ThrowError("You must pass a callback to values().");
    return;
  }

  if (!info[0]->IsFunction()) {
    Nan::ThrowError("You must pass a function as the callback to values().");
    return;
  }

  auto region = Nan::ObjectWrap::Unwrap<Region>(info.Holder());
  auto callback = new Nan::Callback(info[0].As<Function>());

  ValuesWorker* worker = new ValuesWorker(region->region, callback);
  Nan::AsyncQueueWorker(worker);
}

class EntriesWorker : public GemfireWorker {
 public:
  EntriesWorker(const std::shared_ptr<apache::geode::client::Region>& regionPtr,
                Nan::Callback* callback, bool recursive = true)
      : GemfireWorker(callback), regionPtr(regionPtr), recursive(recursive) {}

  void ExecuteGemfireWork() {
    regionEntryVector = regionPtr->entries(recursive);
  }

  void HandleOKCallback() {
    Local<Value> argv[2] = {Nan::Undefined(), v8Value(regionEntryVector)};
    Nan::Call(*callback, 2, argv);
  }

 private:
  std::shared_ptr<apache::geode::client::Region> regionPtr;
  std::vector<std::shared_ptr<apache::geode::client::RegionEntry>>
      regionEntryVector;
  bool recursive;
};

NAN_METHOD(Region::Entries) {
  Nan::HandleScope scope;

  if (info.Length() == 0) {
    Nan::ThrowError("You must pass a callback to entries().");
    return;
  }

  if (!info[0]->IsFunction()) {
    Nan::ThrowError("You must pass a function as the callback to entries().");
    return;
  }

  auto region = Nan::ObjectWrap::Unwrap<Region>(info.Holder());
  auto callback = new Nan::Callback(info[0].As<Function>());

  EntriesWorker* worker = new EntriesWorker(region->region, callback, true);
  Nan::AsyncQueueWorker(worker);
}

class DestroyRegionWorker : public GemfireEventedWorker {
 public:
  DestroyRegionWorker(const Local<Object>& regionObject, Region* region,
                      Nan::Callback* callback, bool local = true)
      : GemfireEventedWorker(regionObject, callback),
        region(region),
        local(local) {}

  void ExecuteGemfireWork() {
    if (local) {
      region->region->localDestroyRegion();
    } else {
      region->region->destroyRegion();
    }
  }

 private:
  Region* region;
  bool local;
};

NAN_METHOD(Region::DestroyRegion) {
  Nan::HandleScope scope;

  if (!isFunctionOrUndefined(info[0])) {
    Nan::ThrowError(
        "You must pass a function as the callback to destroyRegion().");
    return;
  }

  auto region = Nan::ObjectWrap::Unwrap<Region>(info.Holder());

  auto callback = getCallback(info[0]);
  DestroyRegionWorker* worker =
      new DestroyRegionWorker(info.Holder(), region, callback, false);
  Nan::AsyncQueueWorker(worker);

  info.GetReturnValue().Set(info.Holder());
}

NAN_METHOD(Region::LocalDestroyRegion) {
  Nan::HandleScope scope;

  if (!isFunctionOrUndefined(info[0])) {
    Nan::ThrowError(
        "You must pass a function as the callback to localDestroyRegion().");
    return;
  }

  auto region = Nan::ObjectWrap::Unwrap<Region>(info.Holder());

  auto callback = getCallback(info[0]);
  DestroyRegionWorker* worker =
      new DestroyRegionWorker(info.Holder(), region, callback);
  Nan::AsyncQueueWorker(worker);

  info.GetReturnValue().Set(info.Holder());
}

NAN_MODULE_INIT(Region::Init) {
  Nan::HandleScope scope;

  Local<FunctionTemplate> constructorTemplate = Nan::New<FunctionTemplate>();

  constructorTemplate->SetClassName(Nan::New("Region").ToLocalChecked());
  constructorTemplate->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(constructorTemplate, "clear", Region::Clear);
  Nan::SetPrototypeMethod(constructorTemplate, "put", Region::Put);
  Nan::SetPrototypeMethod(constructorTemplate, "putSync", Region::PutSync);
  Nan::SetPrototypeMethod(constructorTemplate, "get", Region::Get);
  Nan::SetPrototypeMethod(constructorTemplate, "getSync", Region::GetSync);
  Nan::SetPrototypeMethod(constructorTemplate, "getAll", Region::GetAll);
  Nan::SetPrototypeMethod(constructorTemplate, "getAllSync",
                          Region::GetAllSync);
  Nan::SetPrototypeMethod(constructorTemplate, "entries", Region::Entries);
  Nan::SetPrototypeMethod(constructorTemplate, "putAll", Region::PutAll);
  Nan::SetPrototypeMethod(constructorTemplate, "putAllSync",
                          Region::PutAllSync);
  Nan::SetPrototypeMethod(constructorTemplate, "remove", Region::Remove);
  Nan::SetPrototypeMethod(constructorTemplate, "query",
                          Region::Query<QueryWorker>);
  Nan::SetPrototypeMethod(constructorTemplate, "selectValue",
                          Region::Query<SelectValueWorker>);
  Nan::SetPrototypeMethod(constructorTemplate, "existsValue",
                          Region::Query<ExistsValueWorker>);
  Nan::SetPrototypeMethod(constructorTemplate, "executeFunction",
                          Region::ExecuteFunction);
  Nan::SetPrototypeMethod(constructorTemplate, "serverKeys",
                          Region::ServerKeys);
  Nan::SetPrototypeMethod(constructorTemplate, "keys", Region::Keys);
  Nan::SetPrototypeMethod(constructorTemplate, "values", Region::Values);
  Nan::SetPrototypeMethod(constructorTemplate, "inspect", Region::Inspect);
  Nan::SetPrototypeMethod(constructorTemplate, "registerAllKeys",
                          Region::RegisterAllKeys);
  Nan::SetPrototypeMethod(constructorTemplate, "unregisterAllKeys",
                          Region::UnregisterAllKeys);
  Nan::SetPrototypeMethod(constructorTemplate, "destroyRegion",
                          Region::DestroyRegion);
  Nan::SetPrototypeMethod(constructorTemplate, "localDestroyRegion",
                          Region::LocalDestroyRegion);

  Nan::SetAccessor(constructorTemplate->InstanceTemplate(),
                   Nan::New<String>("name").ToLocalChecked(), Region::Name);
  Nan::SetAccessor(constructorTemplate->InstanceTemplate(),
                   Nan::New<String>("attributes").ToLocalChecked(),
                   Region::Attributes);

  constructor().Reset(Nan::GetFunction(constructorTemplate).ToLocalChecked());

  Nan::Set(target, Nan::New("Region").ToLocalChecked(),
           Nan::GetFunction(constructorTemplate).ToLocalChecked());
}

}  // namespace node_gemfire
