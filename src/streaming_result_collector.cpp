#include "streaming_result_collector.hpp"

using namespace apache::geode::client;

namespace node_gemfire {

void StreamingResultCollector::addResult(
    const std::shared_ptr<apache::geode::client::Cacheable>&
        resultOfSingleExecution) {
  resultStream->add(resultOfSingleExecution);
}

void StreamingResultCollector::endResults() { resultStream->end(); }

}  // namespace node_gemfire
