package io.pivotal.node_gemfire;

import org.apache.geode.cache.execute.FunctionAdapter;
import org.apache.geode.cache.execute.FunctionContext;

public class ReturnFloat extends FunctionAdapter {

    public void execute(FunctionContext fc) {
        float f = 1.0f;
        fc.getResultSender().lastResult(f);
    }

    public String getId() {
        return getClass().getName();
    }
}
