#ifndef __SELECT_RESULTS_HPP__
#define __SELECT_RESULTS_HPP__

#include <nan.h>
#include <node.h>
#include <v8.h>

#include <geode/SelectResults.hpp>

namespace node_gemfire {

class SelectResults : public Nan::ObjectWrap {
 public:
  explicit SelectResults(
      apache::geode::client::SelectResultsPtr selectResultsPtr)
      : selectResultsPtr(selectResultsPtr) {}

  static NAN_MODULE_INIT(Init);

  static NAN_METHOD(ToArray);
  static NAN_METHOD(Each);
  static NAN_METHOD(Inspect);

  static v8::Local<v8::Object> NewInstance(
      const apache::geode::client::SelectResultsPtr& selectResultsPtr);

 private:
  apache::geode::client::SelectResultsPtr selectResultsPtr;
  static inline Nan::Persistent<v8::Function>& constructor() {
    static Nan::Persistent<v8::Function> my_constructor;
    return my_constructor;
  }
};

}  // namespace node_gemfire

#endif
