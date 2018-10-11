package io.pivotal.node_gemfire;

import org.apache.geode.cache.execute.FunctionAdapter;
import org.apache.geode.cache.execute.FunctionContext;

public class ReturnInteger extends FunctionAdapter {

    public void execute(FunctionContext fc) {
        int i = 1;
        fc.getResultSender().lastResult(i);
    }

    public String getId() {
        return getClass().getName();
    }
}
