#include "result_stream.hpp"

using namespace apache::geode::client;

namespace node_gemfire {

void ResultStream::add(const std::shared_ptr<Cacheable>& resultPtr) {
  uv_mutex_lock(&resultsMutex);
  resultsPtr->push_back(resultPtr);
  uv_mutex_unlock(&resultsMutex);
  uv_async_send(resultsAsync);
}

void ResultStream::end() {
  uv_mutex_lock(&resultsProcessedMutex);
  while (resultsPtr->size() > 0) {
    uv_cond_wait(&resultsProcessedCond, &resultsProcessedMutex);
  }
  uv_mutex_unlock(&resultsProcessedMutex);

  uv_async_send(endAsync);
}

void ResultStream::resultsProcessed() {
  uv_mutex_lock(&resultsProcessedMutex);
  uv_cond_signal(&resultsProcessedCond);
  uv_mutex_unlock(&resultsProcessedMutex);
}

std::shared_ptr<CacheableVector> ResultStream::nextResults() {
  uv_mutex_lock(&resultsMutex);

  auto returnValue = CacheableVector::create();

  for (auto&& iterator : *resultsPtr) {
    returnValue->push_back(iterator);
  }

  resultsPtr->clear();

  uv_mutex_unlock(&resultsMutex);

  return returnValue;
}

void ResultStream::deleteHandle(uv_handle_t* handle) { delete handle; }

}  // namespace node_gemfire
