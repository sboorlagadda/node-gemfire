#include <v8.h>
#include <nan.h>
#include <string>
#include "../../src/conversions.hpp"
#include "../../src/region_shortcuts.hpp"
#include "gtest/gtest.h"

using namespace v8;
using namespace node_gemfire;

TEST(getClassName, emptyObject) {
  Nan::HandleScope scope;

  EXPECT_STREQ(getClassName(Nan::New<Object>()).c_str(),
               getClassName(Nan::New<Object>()).c_str());
}

TEST(getClassName, tinyObject) {
  Nan::HandleScope scope;

  Local<Object> tinyObject = Nan::New<Object>();
  tinyObject->Set(Nan::New("foo").ToLocalChecked(), Nan::New("bar").ToLocalChecked());

  EXPECT_STREQ(getClassName(tinyObject).c_str(),
               getClassName(tinyObject).c_str());

  EXPECT_STRNE(getClassName(tinyObject).c_str(),
               getClassName(Nan::New<Object>()).c_str());
}

TEST(getClassName, valueTypeMatters) {
  Local<Object> firstObject = Nan::New<Object>();
  firstObject->Set(Nan::New("foo").ToLocalChecked(), Nan::New("bar").ToLocalChecked());

  Local<Object> secondObject = Nan::New<Object>();
  secondObject->Set(Nan::New("foo").ToLocalChecked(), Nan::New<Array>());

  EXPECT_STRNE(getClassName(firstObject).c_str(),
               getClassName(secondObject).c_str());
}

TEST(getClassName, indifferentToOrder) {
  Nan::HandleScope scope;

  Local<Object> firstObject = Nan::New<Object>();
  firstObject->Set(Nan::New("foo").ToLocalChecked(), Nan::New("bar").ToLocalChecked());
  firstObject->Set(Nan::New("baz").ToLocalChecked(), Nan::New("qux").ToLocalChecked());

  Local<Object> secondObject = Nan::New<Object>();
  secondObject->Set(Nan::New("baz").ToLocalChecked(), Nan::New("qux").ToLocalChecked());
  secondObject->Set(Nan::New("foo").ToLocalChecked(), Nan::New("bar").ToLocalChecked());

  EXPECT_STREQ(getClassName(firstObject).c_str(),
               getClassName(secondObject).c_str());
}

TEST(getClassName, fieldNamesAreDelimited) {
  Local<Object> firstObject = Nan::New<Object>();
  firstObject->Set(Nan::New("ab").ToLocalChecked(), Nan::Null());
  firstObject->Set(Nan::New("c").ToLocalChecked(), Nan::Null());

  Local<Object> secondObject = Nan::New<Object>();
  secondObject->Set(Nan::New("a").ToLocalChecked(), Nan::Null());
  secondObject->Set(Nan::New("bc").ToLocalChecked(), Nan::Null());

  EXPECT_STRNE(getClassName(firstObject).c_str(),
               getClassName(secondObject).c_str());
}

TEST(getClassName, fieldNamesCanContainCommas) {
  Local<Object> firstObject = Nan::New<Object>();
  firstObject->Set(Nan::New("a,b").ToLocalChecked(), Nan::Null());

  Local<Object> secondObject = Nan::New<Object>();
  secondObject->Set(Nan::New("a").ToLocalChecked(), Nan::Null());
  secondObject->Set(Nan::New("b").ToLocalChecked(), Nan::Null());

  EXPECT_STRNE(getClassName(firstObject).c_str(),
               getClassName(secondObject).c_str());
}

TEST(getClassName, fieldNamesCanContainBrackets) {
  Local<Object> firstObject = Nan::New<Object>();
  firstObject->Set(Nan::New("a").ToLocalChecked(), Nan::New<Array>());

  Local<Object> secondObject = Nan::New<Object>();
  secondObject->Set(Nan::New("a[]").ToLocalChecked(), Nan::Null());

  EXPECT_STRNE(getClassName(firstObject).c_str(),
               getClassName(secondObject).c_str());
}

TEST(getClassName, fieldNamesCanContainBackslash) {
  Local<Object> firstObject = Nan::New<Object>();
  firstObject->Set(Nan::New("a,b").ToLocalChecked(), Nan::Null());

  Local<Object> secondObject = Nan::New<Object>();
  secondObject->Set(Nan::New("a\\").ToLocalChecked(), Nan::Null());
  secondObject->Set(Nan::New("b").ToLocalChecked(), Nan::Null());

  EXPECT_STRNE(getClassName(firstObject).c_str(),
               getClassName(secondObject).c_str());
}

TEST(getRegionShortcut, proxy) {
  EXPECT_EQ(apache::geode::client::PROXY, getRegionShortcut("PROXY"));
}

TEST(getRegionShortcut, cachingProxy) {
  EXPECT_EQ(apache::geode::client::CACHING_PROXY, getRegionShortcut("CACHING_PROXY"));
}

TEST(getRegionShortcut, local) {
  EXPECT_EQ(apache::geode::client::LOCAL, getRegionShortcut("LOCAL"));
}

TEST(getRegionShortcut, cachingProxyEntryLru) {
  EXPECT_EQ(apache::geode::client::CACHING_PROXY_ENTRY_LRU, getRegionShortcut("CACHING_PROXY_ENTRY_LRU"));
}

TEST(getRegionShortcut, localEntryLru) {
  EXPECT_EQ(apache::geode::client::LOCAL_ENTRY_LRU, getRegionShortcut("LOCAL_ENTRY_LRU"));
}

TEST(getRegionShortcut, incorrectShortcut) {
  EXPECT_NE(apache::geode::client::PROXY, getRegionShortcut("NULL"));
  EXPECT_NE(apache::geode::client::CACHING_PROXY, getRegionShortcut("NULL"));
  EXPECT_NE(apache::geode::client::CACHING_PROXY_ENTRY_LRU, getRegionShortcut("NULL"));
  EXPECT_NE(apache::geode::client::LOCAL, getRegionShortcut("NULL"));
  EXPECT_NE(apache::geode::client::LOCAL_ENTRY_LRU, getRegionShortcut("NULL"));
}

NAN_METHOD(run) {
  Nan::HandleScope scope;

  int argc = 0;
  char * argv[0] = {};
  ::testing::InitGoogleTest(&argc, argv);

  int testReturnCode = RUN_ALL_TESTS();

  info.GetReturnValue().Set(Nan::New(testReturnCode));
}

static void Initialize(Local<Object> exports) {
  Nan::SetMethod(exports, "run", run);
}

NODE_MODULE(test, Initialize)
