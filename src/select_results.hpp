#ifndef __SELECT_RESULTS_HPP__
#define __SELECT_RESULTS_HPP__

#include <v8.h>
#include <nan.h>
#include <geode/SelectResults.hpp>
#include <node.h>

namespace node_gemfire {

class SelectResults : public node::ObjectWrap {
 public:
  explicit SelectResults(apache::geode::client::SelectResultsPtr selectResultsPtr) :
    selectResultsPtr(selectResultsPtr) {}

  static void Init(v8::Local<v8::Object> exports);
  static v8::Local<v8::Object> NewInstance(
      const apache::geode::client::SelectResultsPtr & selectResultsPtr);
  static NAN_METHOD(ToArray);
  static NAN_METHOD(Each);
  static NAN_METHOD(Inspect);

 private:
  apache::geode::client::SelectResultsPtr selectResultsPtr;
  static v8::Persistent<v8::Function> constructor;
};


}  // namespace node_gemfire

#endif
