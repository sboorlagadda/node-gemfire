package io.pivotal.node_gemfire;

import org.apache.geode.cache.execute.FunctionAdapter;
import org.apache.geode.cache.execute.FunctionContext;

public class ReturnNull extends FunctionAdapter {

    public void execute(FunctionContext fc) {
        fc.getResultSender().lastResult(null);
    }

    public String getId() {
        return getClass().getName();
    }
}
