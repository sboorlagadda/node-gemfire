#include "streaming_result_collector.hpp"

using namespace apache::geode::client;

namespace node_gemfire {

void StreamingResultCollector::addResult(CacheablePtr & resultPtr) {
  resultStream->add(resultPtr);
}

void StreamingResultCollector::endResults() {
  resultStream->end();
}

}  // namespace node_gemfire

