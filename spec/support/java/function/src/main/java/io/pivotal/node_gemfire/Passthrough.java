package io.pivotal.node_gemfire;

import org.apache.geode.cache.execute.FunctionAdapter;
import org.apache.geode.cache.execute.FunctionContext;

public class Passthrough extends FunctionAdapter {

    public void execute(FunctionContext fc) {
        Object args = fc.getArguments();

        if(args == null) {
            fc.getResultSender().sendException(new Exception("Expected arguments; no arguments received"));
            return;
        }

        fc.getResultSender().lastResult(args);
    }

    public String getId() {
        return getClass().getName();
    }
}
