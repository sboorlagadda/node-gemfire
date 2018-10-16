#ifndef __CACHE_HPP__
#define __CACHE_HPP__

#include <nan.h>
#include <node.h>
#include <v8.h>

#include <geode/Cache.hpp>
#include <memory>

namespace node_gemfire {

class Cache : public Nan::ObjectWrap {
 public:
  // Called from binding.cpp to initialize the system
  static void Init(v8::Local<v8::Object> exports);

  std::unique_ptr<apache::geode::client::Cache> cache;
  static v8::Local<v8::Object> NewInstance(std::unique_ptr<apache::geode::client::Cache>);
 
 protected:
  explicit Cache(
      std::unique_ptr<apache::geode::client::Cache> cache) :
    cache(std::move(cache)) {}

  virtual ~Cache() { close(); }

  void close();

  static NAN_METHOD(Close);
  static NAN_METHOD(ExecuteFunction);
  static NAN_METHOD(ExecuteQuery);
  static NAN_METHOD(CreateRegion);
  static NAN_METHOD(GetRegion);
  static NAN_METHOD(RootRegions);
  static NAN_METHOD(Inspect);

 private:
  static std::shared_ptr<apache::geode::client::Pool> getPool(const v8::Handle<v8::Value> & poolNameValue);
  static v8::Local<v8::Function> exitCallback();

  static inline Nan::Persistent<v8::Function>& constructor() {
    static Nan::Persistent<v8::Function> my_constructor;
    return my_constructor;
  }
};

}  // namespace node_gemfire

#endif
