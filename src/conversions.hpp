#ifndef __CONVERSIONS_HPP__
#define __CONVERSIONS_HPP__

#include <nan.h>
#include <sys/time.h>
#include <v8.h>

#include <cstdint>
#include <string>

#include <geode/CacheFactory.hpp>
#include <geode/CacheableBuiltins.hpp>
#include <geode/PdxInstanceFactory.hpp>
#include <geode/RegionEntry.hpp>
#include <geode/Struct.hpp>

namespace node_gemfire {

std::shared_ptr<apache::geode::client::Cacheable> gemfireValue(
    const v8::Local<v8::Value>& v8Value,
    apache::geode::client::Cache& cachePtr);
std::shared_ptr<apache::geode::client::PdxInstance> gemfireValue(
    const v8::Local<v8::Object>& v8Object,
    apache::geode::client::Cache& cachePtr);
std::shared_ptr<apache::geode::client::CacheableArrayList> gemfireValue(
    const v8::Local<v8::Array>& v8Value,
    apache::geode::client::Cache& cachePtr);
std::shared_ptr<apache::geode::client::CacheableDate> gemfireValue(
    const v8::Local<v8::Date>& v8Value);

std::shared_ptr<apache::geode::client::CacheableKey> gemfireKey(
    const v8::Local<v8::Value>& v8Value,
    apache::geode::client::Cache& cachePtr);
std::vector<std::shared_ptr<apache::geode::client::CacheableKey>> gemfireKeys(
    const v8::Local<v8::Array>& v8Value,
    apache::geode::client::Cache& cachePtr);

apache::geode::client::HashMapOfCacheable gemfireHashMap(
    const v8::Local<v8::Object>& v8Object,
    apache::geode::client::Cache& cachePtr);
std::shared_ptr<apache::geode::client::CacheableVector> gemfireVector(
    const v8::Local<v8::Array>& v8Array,
    apache::geode::client::Cache& cachePtr);

v8::Local<v8::Value> v8Value(
    const std::shared_ptr<apache::geode::client::Cacheable>& valuePtr);
v8::Local<v8::Value> v8Value(
    const std::shared_ptr<apache::geode::client::CacheableKey>& keyPtr);
v8::Local<v8::Value> v8Value(
    const std::shared_ptr<apache::geode::client::CacheableInt64>& valuePtr);
v8::Local<v8::Object> v8Value(
    const std::shared_ptr<apache::geode::client::Struct>& structPtr);
v8::Local<v8::Value> v8Value(
    const std::shared_ptr<apache::geode::client::PdxInstance>& pdxInstancePtr);
v8::Local<v8::Object> v8Value(
    const std::shared_ptr<apache::geode::client::SelectResults>&
        selectResultsPtr);
v8::Local<v8::Object> v8Value(
    const std::shared_ptr<apache::geode::client::CacheableHashMap>& hashMapPtr);
v8::Local<v8::Object> v8Value(
    const apache::geode::client::HashMapOfCacheable& hashMapPtr);
v8::Local<v8::Object> v8Value(
    const std::shared_ptr<apache::geode::client::RegionEntry>& regionEntryPtr);
v8::Local<v8::Array> v8Value(
    const std::vector<std::shared_ptr<apache::geode::client::Cacheable>>&
        vectorPtr);
v8::Local<v8::Array> v8Value(
    const std::vector<std::shared_ptr<apache::geode::client::CacheableKey>>&
        vectorPtr);
v8::Local<v8::Array> v8Value(
    const std::vector<std::shared_ptr<apache::geode::client::RegionEntry>>&
        vectorPtr);
v8::Local<v8::Date> v8Value(
    const std::shared_ptr<apache::geode::client::CacheableDate>& datePtr);
v8::Local<v8::Boolean> v8Value(bool value);

template <typename T>
v8::Local<v8::Array> v8Array(const T& iterable) {
  Nan::EscapableHandleScope scope;

  auto length = iterable.size();
  auto v8Array = Nan::New<v8::Array>(length);

  decltype(length) i = 0;
  for (auto&& iterator : iterable) {
    v8Array->Set(i, v8Value(iterator));
    i++;
  }

  return scope.Escape(v8Array);
}

template <typename... T>
v8::Local<v8::Object> v8Object(const std::unordered_map<T...>& hashMap) {
  Nan::EscapableHandleScope scope;
  auto v8Object = Nan::New<v8::Object>();

  for (auto&& iterator : hashMap) {
    auto&& key = iterator.first;
    auto&& value = iterator.second;

    v8Object->Set(v8Value(key), v8Value(value));
  }

  return scope.Escape(v8Object);
}

template <typename T>
v8::Local<v8::Array> v8Array(const std::shared_ptr<T>& iterable) {
  return v8Array(*iterable);
}

template <typename T>
v8::Local<v8::Object> v8Object(const std::shared_ptr<T>& hashMap) {
  return v8Object(*hashMap);
}

std::string getClassName(const v8::Local<v8::Object>& v8Object);

}  // namespace node_gemfire

#endif
