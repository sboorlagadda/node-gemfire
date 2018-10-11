package io.pivotal.node_gemfire;

import org.apache.geode.cache.execute.FunctionAdapter;
import org.apache.geode.cache.execute.FunctionContext;

public class ReturnShort extends FunctionAdapter {

    public void execute(FunctionContext fc) {
        short s = 1;
        fc.getResultSender().lastResult(s);
    }

    public String getId() {
        return getClass().getName();
    }
}
