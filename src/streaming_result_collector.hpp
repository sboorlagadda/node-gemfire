#ifndef __STREAMING_RESULT_COLLECTOR_HPP__
#define __STREAMING_RESULT_COLLECTOR_HPP__

#include <geode/ResultCollector.hpp>

#include "result_stream.hpp"

namespace node_gemfire {

class StreamingResultCollector : public apache::geode::client::ResultCollector {
 public:
  explicit StreamingResultCollector(ResultStream* resultStream)
      : ResultCollector(), resultStream(resultStream) {}

  virtual std::shared_ptr<apache::geode::client::CacheableVector> getResult(
      std::chrono::milliseconds) override {
    return nullptr;
  }

  void addResult(const std::shared_ptr<apache::geode::client::Cacheable>&
                     resultOfSingleExecution) override;

  void endResults() override;

  void clearResults() override {}

 private:
  ResultStream* resultStream;
};

}  // namespace node_gemfire

#endif
