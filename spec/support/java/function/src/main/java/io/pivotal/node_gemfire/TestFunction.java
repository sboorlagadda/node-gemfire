package io.pivotal.node_gemfire;

import org.apache.geode.cache.execute.FunctionAdapter;
import org.apache.geode.cache.execute.FunctionContext;

public class TestFunction extends FunctionAdapter {

    public void execute(FunctionContext fc) {
        fc.getResultSender().lastResult("TestFunction succeeded.");
    }

    public String getId() {
        return getClass().getName();
    }
}
