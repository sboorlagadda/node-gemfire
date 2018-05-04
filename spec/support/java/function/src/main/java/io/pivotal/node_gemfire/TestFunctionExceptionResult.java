package io.pivotal.node_gemfire;

import org.apache.geode.cache.execute.FunctionAdapter;
import org.apache.geode.cache.execute.FunctionContext;
import org.apache.geode.cache.execute.FunctionException;

public class TestFunctionExceptionResult extends FunctionAdapter {

    public void execute(FunctionContext fc) {
        fc.getResultSender().sendResult("First result");
        fc.getResultSender().sendException(new Exception("Test exception message sent by server."));
    }

    public String getId() {
        return getClass().getName();
    }
}