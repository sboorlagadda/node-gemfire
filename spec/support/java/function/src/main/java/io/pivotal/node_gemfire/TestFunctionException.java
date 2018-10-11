package io.pivotal.node_gemfire;

import org.apache.geode.cache.execute.FunctionAdapter;
import org.apache.geode.cache.execute.FunctionContext;
import org.apache.geode.cache.execute.FunctionException;
import org.apache.geode.cache.execute.ResultSender;

public class TestFunctionException extends FunctionAdapter {

    public void execute(FunctionContext fc) {
        throw new FunctionException("Test exception message thrown by server.");
    }

    public String getId() {
        return getClass().getName();
    }
}