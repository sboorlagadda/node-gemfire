package io.pivotal.node_gemfire;

import org.apache.geode.cache.execute.FunctionAdapter;
import org.apache.geode.cache.execute.FunctionContext;

import java.util.HashSet;

public class ReturnSet extends FunctionAdapter {

    public void execute(FunctionContext fc) {
        HashSet<String> hs = new HashSet<String>();
        hs.add("foo");
        hs.add("bar");
        fc.getResultSender().lastResult(hs);
    }

    public String getId() {
        return getClass().getName();
    }
}
