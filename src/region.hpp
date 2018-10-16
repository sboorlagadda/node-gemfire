#ifndef __REGION_HPP__
#define __REGION_HPP__

#include <nan.h>
#include <node.h>
#include <v8.h>

#include <geode/Region.hpp>

#include "region_event_registry.hpp"

namespace node_gemfire {

class Region : public Nan::ObjectWrap {
 public:
  Region(std::shared_ptr<apache::geode::client::Region> region) :
    region(region) {}

  virtual ~Region() { RegionEventRegistry::getInstance()->remove(this); }

  static NAN_MODULE_INIT(Init);
  static v8::Local<v8::Object> NewInstance(std::shared_ptr<apache::geode::client::Region>);

  static NAN_METHOD(Clear);
  static NAN_METHOD(Put);
  static NAN_METHOD(PutSync);
  static NAN_METHOD(Get);
  static NAN_METHOD(GetSync);
  static NAN_METHOD(GetAll);
  static NAN_METHOD(GetAllSync);
  static NAN_METHOD(Entries);
  static NAN_METHOD(PutAll);
  static NAN_METHOD(PutAllSync);
  static NAN_METHOD(Remove);
  static NAN_METHOD(ServerKeys);
  static NAN_METHOD(Keys);
  static NAN_METHOD(Values);
  static NAN_METHOD(ExecuteFunction);
  static NAN_METHOD(RegisterAllKeys);
  static NAN_METHOD(UnregisterAllKeys);
  static NAN_METHOD(DestroyRegion);
  static NAN_METHOD(LocalDestroyRegion);
  static NAN_METHOD(Inspect);
  static NAN_GETTER(Name);
  static NAN_GETTER(Attributes);

  template <typename T>
  static NAN_METHOD(Query);

  std::shared_ptr<apache::geode::client::Region> region;

 private:
  static inline Nan::Persistent<v8::Function>& constructor() {
    static Nan::Persistent<v8::Function> my_constructor;
    return my_constructor;
  }
};

}  // namespace node_gemfire

#endif
