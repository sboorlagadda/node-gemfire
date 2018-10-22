#include "conversions.hpp"

#include <math.h>
#include <nan.h>
#include <node.h>
#include <v8.h>

#include <cstdint>
#include <set>
#include <sstream>
#include <string>

#include <geode/CacheableUndefined.hpp>
#include <geode/internal/DataSerializablePrimitive.hpp>

#include "exceptions.hpp"
#include "select_results.hpp"

using namespace std;
using namespace chrono;
using namespace v8;

namespace node_gemfire {

std::string getClassName(const Local<Object>& v8Object) {
  Nan::HandleScope scope;

  std::set<std::string> fieldNames;
  uint totalSize = 0;

  Local<Array> v8Keys(v8Object->GetOwnPropertyNames());
  unsigned int numKeys = v8Keys->Length();
  for (unsigned int i = 0; i < numKeys; i++) {
    Local<Value> v8Key(v8Keys->Get(i));
    Nan::Utf8String utf8FieldName(v8Key);
    char* fieldName = *utf8FieldName;

    unsigned int size = utf8FieldName.length();
    std::string fullFieldName;
    fullFieldName.reserve((size * 2) +
                          3);  // escape every character, plus '[],'

    for (unsigned int j = 0; j < size; j++) {
      char fieldNameChar = fieldName[j];
      switch (fieldNameChar) {
        case ',':
        case '[':
        case ']':
        case '\\':
          fullFieldName += '\\';
      }
      fullFieldName += fieldNameChar;
    }

    Local<Value> v8Value(v8Object->Get(v8Key));
    if (v8Value->IsArray() && !v8Value->IsString()) {
      fullFieldName += "[]";
    }
    fullFieldName += ',';

    fieldNames.insert(fullFieldName);
    totalSize += fullFieldName.length();
  }

  std::string className;
  className.reserve(totalSize + 7);
  className += "JSON: ";

  for (std::set<std::string>::iterator i(fieldNames.begin());
       i != fieldNames.end(); ++i) {
    className += *i;
  }
  return className;
}

std::wstring wstringFromV8String(const Local<String>& v8String) {
  Nan::HandleScope scope;

  String::Value v8StringValue(v8String);
  uint16_t* v8StringData(*v8StringValue);

  unsigned int length = v8String->Length();
  wchar_t* buffer = new wchar_t[length + 1];
  for (unsigned int i = 0; i < length; i++) {
    buffer[i] = v8StringData[i];
  }
  buffer[length] = 0;

  std::wstring wstring(buffer);
  delete[] buffer;

  return wstring;
}

Local<String> v8StringFromWstring(const std::wstring& wideString) {
  Nan::EscapableHandleScope scope;

  unsigned int length = wideString.length();
  uint16_t* buffer = new uint16_t[length + 1];
  for (unsigned int i = 0; i < length; i++) {
    buffer[i] = wideString[i];
  }
  buffer[length] = 0;

  Local<String> v8String = Nan::New(buffer).ToLocalChecked();
  delete[] buffer;

  return scope.Escape(v8String);
}

void ConsoleWarn(const char* message) {
  Nan::HandleScope scope;

  auto global = Nan::GetCurrentContext()->Global();
  auto consoleObject =
      global->Get(Nan::New("console").ToLocalChecked()).As<Object>();
  auto warnFunction =
      consoleObject->Get(Nan::New("warn").ToLocalChecked()).As<Function>();
  Local<Value> argv[1] = {Nan::New(message).ToLocalChecked()};
  Nan::Callback callback(warnFunction);
  callback.Call(1, argv);
}

std::shared_ptr<apache::geode::client::Cacheable> gemfireValue(
    const Local<Value>& v8Value, apache::geode::client::Cache& cachePtr) {
  if (v8Value->IsString() || v8Value->IsStringObject()) {
    return apache::geode::client::CacheableString::create(
        *Nan::Utf8String(v8Value->ToString()));
  } else if (v8Value->IsBoolean()) {
    return apache::geode::client::CacheableBoolean::create(
        v8Value->ToBoolean()->Value());
  } else if (v8Value->IsNumber() || v8Value->IsNumberObject()) {
    return apache::geode::client::CacheableDouble::create(
        v8Value->ToNumber(Isolate::GetCurrent()->GetCurrentContext()).FromMaybe(Local<Number>())->Value());
  } else if (v8Value->IsDate()) {
    return gemfireValue(Local<Date>::Cast(v8Value));
  } else if (v8Value->IsArray()) {
    return gemfireValue(Local<Array>::Cast(v8Value), cachePtr);
  } else if (v8Value->IsBooleanObject()) {
#if (NODE_MODULE_VERSION > 0x000B)
    return apache::geode::client::CacheableBoolean::create(
        BooleanObject::Cast(*v8Value)->ValueOf());
#else
    return CacheableBoolean::create(
        BooleanObject::Cast(*v8Value)->BooleanValue());
#endif
  } else if (v8Value->IsFunction()) {
    Nan::ThrowError(
        "Unable to serialize to GemFire; functions are not supported.");
    return nullptr;
  } else if (v8Value->IsObject()) {
    return gemfireValue(v8Value->ToObject(), cachePtr);
  } else if (v8Value->IsUndefined()) {
    return apache::geode::client::CacheableUndefined::create();
  } else if (v8Value->IsNull()) {
    return nullptr;
  } else {
    std::string errorMessage(
        "Unable to serialize to GemFire; unknown JavaScript object: ");
    errorMessage.append(*Nan::Utf8String(v8Value->ToDetailString(Isolate::GetCurrent()->GetCurrentContext()).FromMaybe(Local<String>())));
    Nan::ThrowError(errorMessage.c_str());
    return nullptr;
  }
}

std::shared_ptr<apache::geode::client::PdxInstance> gemfireValue(
    const Local<Object>& v8Object, apache::geode::client::Cache& cachePtr) {
  Nan::EscapableHandleScope scope;
  try {
    std::string pdxClassName = getClassName(v8Object);
    auto pdxInstanceFactory = cachePtr.createPdxInstanceFactory(pdxClassName);
    Local<Array> v8Keys(v8Object->GetOwnPropertyNames());
    unsigned int length = v8Keys->Length();
    for (unsigned int i = 0; i < length; i++) {
      Local<Value> v8Key(v8Keys->Get(i));
      Local<Value> v8Value(v8Object->Get(v8Key));
      Nan::Utf8String fieldName(v8Key);
      auto cacheablePtr = gemfireValue(v8Value, cachePtr);
      pdxInstanceFactory.writeObject(*fieldName, cacheablePtr);
    }
    return pdxInstanceFactory.create();
  } catch (const apache::geode::client::Exception& exception) {
    ThrowGemfireException(exception);
    return nullptr;
  }
}

std::shared_ptr<apache::geode::client::CacheableArrayList> gemfireValue(
    const Local<Array>& v8Array, apache::geode::client::Cache& cachePtr) {
  auto arrayListPtr = apache::geode::client::CacheableArrayList::create();
  auto length = v8Array->Length();
  for (decltype(length) i = 0; i < length; i++) {
    arrayListPtr->push_back(gemfireValue(v8Array->Get(i), cachePtr));
  }
  return arrayListPtr;
}

std::shared_ptr<apache::geode::client::CacheableDate> gemfireValue(
    const Local<Date>& v8Date) {
  auto millisecondsSinceEpoch = v8Date->IntegerValue();
  auto time = std::chrono::system_clock::time_point(
      std::chrono::milliseconds(millisecondsSinceEpoch));
  return apache::geode::client::CacheableDate::create(time);
}

std::shared_ptr<apache::geode::client::CacheableKey> gemfireKey(
    const Local<Value>& v8Value, apache::geode::client::Cache& cachePtr) {
  std::shared_ptr<apache::geode::client::CacheableKey> keyPtr;
  try {
    keyPtr = std::dynamic_pointer_cast<apache::geode::client::CacheableKey>(
        gemfireValue(v8Value, cachePtr));
  } catch (const apache::geode::client::ClassCastException& exception) {
    return nullptr;
  }

  return keyPtr;
}

std::vector<shared_ptr<apache::geode::client::CacheableKey>> gemfireKeys(
    const Local<Array>& v8Value, apache::geode::client::Cache& cachePtr) {
  auto length = v8Value->Length();
  std::vector<shared_ptr<apache::geode::client::CacheableKey>> vectorPtr;
  vectorPtr.reserve(length);

  for (decltype(length) i = 0; i < length; i++) {
    auto keyPtr = gemfireKey(v8Value->Get(i), cachePtr);
    vectorPtr.push_back(keyPtr);
  }

  return vectorPtr;
}

apache::geode::client::HashMapOfCacheable gemfireHashMap(
    const Local<Object>& v8Object, apache::geode::client::Cache& cachePtr) {
  Nan::HandleScope scope;

  auto v8Keys = v8Object->GetOwnPropertyNames();
  auto length = v8Keys->Length();

  apache::geode::client::HashMapOfCacheable hashMapPtr;
  hashMapPtr.reserve(length);

  for (decltype(length) i = 0; i < length; i++) {
    auto v8Key = v8Keys->Get(i)->ToString();

    auto keyPtr = gemfireKey(v8Key, cachePtr);
    auto valuePtr = gemfireValue(v8Object->Get(v8Key), cachePtr);

    hashMapPtr.emplace(keyPtr, valuePtr);
  }

  return hashMapPtr;
}

std::shared_ptr<apache::geode::client::CacheableVector> gemfireVector(
    const Local<Array>& v8Array, apache::geode::client::Cache& cachePtr) {
  Nan::HandleScope scope;

  auto length = v8Array->Length();
  auto vectorPtr = apache::geode::client::CacheableVector::create();

  for (decltype(length) i = 0; i < length; i++) {
    vectorPtr->push_back(gemfireValue(v8Array->Get(i), cachePtr));
  }

  return vectorPtr;
}

Local<Value> v8Value(
    const std::shared_ptr<apache::geode::client::Cacheable>& valuePtr) {
  Nan::EscapableHandleScope scope;

  if (valuePtr == nullptr) {
    return scope.Escape(Nan::Null());
  }

  using namespace apache::geode::client;
  using namespace apache::geode::client::internal;

  if (auto dataSerializablePrimitive =
          std::dynamic_pointer_cast<DataSerializablePrimitive>(valuePtr)) {
    auto dsCode = dataSerializablePrimitive->getDsCode();
    switch (dsCode) {
      case DSCode::CacheableASCIIString:
      case DSCode::CacheableASCIIStringHuge:
      case DSCode::CacheableString:
      case DSCode::CacheableStringHuge: {
        auto cacheableStringPtr =
            std::dynamic_pointer_cast<CacheableString>(valuePtr);
        return scope.Escape(
            Nan::New(cacheableStringPtr->value()).ToLocalChecked());
      }
      case DSCode::CacheableBoolean:
        return scope.Escape(Nan::New(
            (std::dynamic_pointer_cast<CacheableBoolean>(valuePtr))->value()));
      case DSCode::CacheableDouble:
        return scope.Escape(Nan::New(
            (std::dynamic_pointer_cast<CacheableDouble>(valuePtr))->value()));
      case DSCode::CacheableFloat:
        return scope.Escape(Nan::New(
            (std::dynamic_pointer_cast<CacheableFloat>(valuePtr))->value()));
      case DSCode::CacheableInt16:
        return scope.Escape(Nan::New(
            (std::dynamic_pointer_cast<CacheableInt16>(valuePtr))->value()));
      case DSCode::CacheableInt32:
        return scope.Escape(Nan::New(
            (std::dynamic_pointer_cast<CacheableInt32>(valuePtr))->value()));
      case DSCode::CacheableInt64:
        return scope.Escape(
            v8Value(std::dynamic_pointer_cast<CacheableInt64>(valuePtr)));
      case DSCode::CacheableDate:
        return scope.Escape(
            v8Value(std::dynamic_pointer_cast<CacheableDate>(valuePtr)));
      case DSCode::CacheableObjectArray:
        return scope.Escape(
            v8Array(std::dynamic_pointer_cast<CacheableObjectArray>(valuePtr)));
      case DSCode::CacheableArrayList:
        return scope.Escape(
            v8Array(std::dynamic_pointer_cast<CacheableArrayList>(valuePtr)));
      case DSCode::CacheableVector:
        return scope.Escape(
            v8Array(std::dynamic_pointer_cast<CacheableVector>(valuePtr)));
      case DSCode::CacheableHashMap:
        return scope.Escape(
            v8Object(std::dynamic_pointer_cast<CacheableHashMap>(valuePtr)));
      case DSCode::CacheableHashSet:
        return scope.Escape(
            v8Array(std::dynamic_pointer_cast<CacheableHashSet>(valuePtr)));
      default:
        std::stringstream errorMessageStream;
        errorMessageStream
            << "Unable to serialize value from GemFire; unknown DSCode: "
            << static_cast<int8_t>(dsCode);
        Nan::ThrowError(errorMessageStream.str().c_str());
    }
  } else if (auto dataSerializableFixedId =
                 std::dynamic_pointer_cast<DataSerializableFixedId>(valuePtr)) {
    auto fixedId = dataSerializableFixedId->getDSFID();
    switch (fixedId) {
      case DSFid::CacheableUndefined:
        return scope.Escape(Nan::Undefined());
      case DSFid::Struct:
        return scope.Escape(
            v8Value(std::dynamic_pointer_cast<Struct>(valuePtr)));
      default:
        std::stringstream errorMessageStream;
        errorMessageStream
            << "Unable to serialize value from GemFire; unknown FixedId: "
            << static_cast<int32_t>(fixedId);
        Nan::ThrowError(errorMessageStream.str().c_str());
    }
  } else if (auto userFunctionExecutionException =
                 std::dynamic_pointer_cast<UserFunctionExecutionException>(
                     valuePtr)) {
    return scope.Escape(v8Error(userFunctionExecutionException));
  } else if (auto pdxInstance =
                 std::dynamic_pointer_cast<PdxInstance>(valuePtr)) {
    return scope.Escape(v8Value(pdxInstance));
  }

  std::stringstream errorMessageStream;
  errorMessageStream << "Unable to serialize value from GemFire: "
                     << valuePtr->toString();
  Nan::ThrowError(errorMessageStream.str().c_str());
  return scope.Escape(Nan::Undefined());
}

Local<Value> v8Value(
    const std::shared_ptr<apache::geode::client::PdxInstance>& pdxInstance) {
  Nan::EscapableHandleScope scope;

  try {
    auto gemfireKeys = pdxInstance->getFieldNames();

    if (gemfireKeys == nullptr) {
      return scope.Escape(Nan::New<Object>());
    }

    auto v8Object = Nan::New<Object>();
    auto length = gemfireKeys->length();

    for (decltype(length) i = 0; i < length; i++) {
      auto key = (*gemfireKeys)[i]->value();
      std::shared_ptr<apache::geode::client::Cacheable> value;
      if (pdxInstance->getFieldType(key) ==
          apache::geode::client::PdxFieldTypes::OBJECT_ARRAY) {
        value = pdxInstance->getCacheableObjectArrayField(key);
      } else {
        value = pdxInstance->getCacheableField(key);
      }
      Nan::Set(v8Object, Nan::New(key).ToLocalChecked(), v8Value(value));
    }

    return scope.Escape(v8Object);
  } catch (const apache::geode::client::Exception& exception) {
    ThrowGemfireException(exception);
    Nan::EscapableHandleScope scope;
    return scope.Escape(Nan::Undefined());
  }
}

Local<Value> v8Value(
    const std::shared_ptr<apache::geode::client::CacheableInt64>& valuePtr) {
  Nan::EscapableHandleScope scope;

  static const int64_t maxSafeInteger = pow(2, 53) - 1;
  static const int64_t minSafeInteger = -1 * maxSafeInteger;

  auto value =
      static_cast<std::shared_ptr<apache::geode::client::CacheableInt64>>(
          valuePtr)
          ->value();
  if (value > maxSafeInteger) {
    ConsoleWarn(
        "Received 64 bit integer from GemFire greater than "
        "Number.MAX_SAFE_INTEGER (2^53 - 1)");
  } else if (value < minSafeInteger) {
    ConsoleWarn(
        "Received 64 bit integer from GemFire less than "
        "Number.MIN_SAFE_INTEGER (-1 * 2^53 + 1)");
  }

  return scope.Escape(Nan::New<Number>(value));
}

Local<Date> v8Value(
    const std::shared_ptr<apache::geode::client::CacheableDate>& datePtr) {
  Nan::EscapableHandleScope scope;
  double epochMillis = datePtr->milliseconds();
  return scope.Escape(Nan::New<Date>(epochMillis).ToLocalChecked());
}

Local<Value> v8Value(
    const std::shared_ptr<apache::geode::client::CacheableKey>& keyPtr) {
  return v8Value(
      static_cast<std::shared_ptr<apache::geode::client::Cacheable>>(keyPtr));
}

Local<Object> v8Value(
    const std::shared_ptr<apache::geode::client::Struct>& structPtr) {
  Nan::EscapableHandleScope scope;
  Local<Object> v8Object(Nan::New<Object>());
  auto length = structPtr->size();
  for (decltype(length) i = 0; i < length; i++) {
    Nan::Set(v8Object, Nan::New(structPtr->getFieldName(i)).ToLocalChecked(),
             v8Value((*structPtr)[i]));
  }
  return scope.Escape(v8Object);
}

Local<Object> v8Value(
    const apache::geode::client::HashMapOfCacheable& hashMapPtr) {
  return v8Object(hashMapPtr);
}

Local<Array> v8Value(
    const std::vector<std::shared_ptr<apache::geode::client::Cacheable>>&
        vectorPtr) {
  return v8Array(vectorPtr);
}

Local<Array> v8Value(
    const std::vector<std::shared_ptr<apache::geode::client::CacheableKey>>&
        vectorPtr) {
  return v8Array(vectorPtr);
}

Local<Object> v8Value(
    const std::shared_ptr<apache::geode::client::RegionEntry>& regionEntryPtr) {
  Nan::EscapableHandleScope scope;
  auto v8Object = Nan::New<Object>();
  Nan::Set(v8Object, Nan::New("key").ToLocalChecked(),
           v8Value(regionEntryPtr->getKey()));
  Nan::Set(v8Object, Nan::New("value").ToLocalChecked(),
           v8Value(regionEntryPtr->getValue()));
  return scope.Escape(v8Object);
}

Local<Array> v8Value(
    const std::vector<std::shared_ptr<apache::geode::client::RegionEntry>>&
        vectorOfRegionEntries) {
  Nan::EscapableHandleScope scope;
  auto v8Array = Nan::New<Array>();
  auto length = vectorOfRegionEntries.size();
  for (decltype(length) i = 0; i < length; i++) {
    Nan::Set(v8Array, i, v8Value(vectorOfRegionEntries[i]));
  }
  return scope.Escape(v8Array);
}

Local<Object> v8Value(
    const std::shared_ptr<apache::geode::client::SelectResults>&
        selectResultsPtr) {
  Nan::EscapableHandleScope scope;

  Local<Object> selectResults(SelectResults::NewInstance(selectResultsPtr));

  return scope.Escape(selectResults);
}

Local<Boolean> v8Value(bool value) {
  Nan::EscapableHandleScope scope;
  return scope.Escape(Nan::New(value));
}

}  // namespace node_gemfire
