#ifndef __RESULT_STREAM_HPP__
#define __RESULT_STREAM_HPP__

#include <uv.h>

#include <geode/CacheableBuiltins.hpp>

namespace node_gemfire {

class ResultStream {
 public:
  explicit ResultStream(void *worker, uv_async_cb resultsCallback,
                        uv_async_cb endCallback)
      : resultsAsync(new uv_async_t),
        endAsync(new uv_async_t),
        resultsPtr(apache::geode::client::CacheableVector::create()) {
    uv_mutex_init(&resultsMutex);
    uv_mutex_init(&resultsProcessedMutex);
    uv_cond_init(&resultsProcessedCond);
    resultsAsync->data = worker;
    endAsync->data = worker;
    uv_async_init(uv_default_loop(), resultsAsync, resultsCallback);
    uv_async_init(uv_default_loop(), endAsync, endCallback);
  }

  ~ResultStream() {
    uv_close(reinterpret_cast<uv_handle_t *>(endAsync), deleteHandle);
    uv_close(reinterpret_cast<uv_handle_t *>(resultsAsync), deleteHandle);
    uv_mutex_destroy(&resultsMutex);
    uv_mutex_destroy(&resultsProcessedMutex);
    uv_cond_destroy(&resultsProcessedCond);
  }

  void add(const std::shared_ptr<apache::geode::client::Cacheable> &resultPtr);
  void end();
  void resultsProcessed();

  std::shared_ptr<apache::geode::client::CacheableVector> nextResults();

 private:
  static void deleteHandle(uv_handle_t *handle);

  uv_mutex_t resultsMutex;
  uv_mutex_t resultsProcessedMutex;

  uv_async_t *resultsAsync;
  uv_async_t *endAsync;

  uv_cond_t resultsProcessedCond;

  std::shared_ptr<apache::geode::client::CacheableVector> resultsPtr;
};

}  // namespace node_gemfire

#endif
