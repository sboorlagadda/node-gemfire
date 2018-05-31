#ifndef __GEMFIRE_WORKER_HPP__
#define __GEMFIRE_WORKER_HPP__

#include <nan.h>
#include <geode/GeodeCppCache.hpp>
#include <string>

namespace node_gemfire {

class GemfireWorker : public Nan::AsyncWorker {
 public:
    explicit GemfireWorker(Nan::Callback * callback) :
      Nan::AsyncWorker(callback),
      errorName(),
      threwException(false) {}

    void Execute();
    virtual void ExecuteGemfireWork() = 0;
    void WorkComplete();
    void SetError(const char * name, const char * message);
  
  protected: 
    v8::Local<v8::Value> errorObject();
    std::string errorName;
    bool threwException;
};

}  // namespace node_gemfire

#endif
