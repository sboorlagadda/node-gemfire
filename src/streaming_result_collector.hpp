#ifndef __STREAMING_RESULT_COLLECTOR_HPP__
#define __STREAMING_RESULT_COLLECTOR_HPP__

#include <geode/ResultCollector.hpp>
#include "result_stream.hpp"

namespace node_gemfire {

class StreamingResultCollector : public apache::geode::client::ResultCollector {
 public:
  explicit StreamingResultCollector(ResultStream * resultStream) :
      ResultCollector(),
      resultStream(resultStream) {}

  virtual void addResult(apache::geode::client::CacheablePtr & resultPtr);
  virtual void endResults();

 private:
  ResultStream * resultStream;
};

}  // namespace node_gemfire

#endif
