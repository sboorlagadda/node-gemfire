package io.pivotal.node_gemfire;

import org.apache.geode.cache.execute.FunctionAdapter;
import org.apache.geode.cache.execute.FunctionContext;

import java.util.List;

public class Sum extends FunctionAdapter {

    public void execute(FunctionContext fc) {
        List addendObjects = (List) fc.getArguments();

        Double sum = 0.0;
        for(Object addendObject : addendObjects) {
            sum += (Double) addendObject;
        }

        fc.getResultSender().lastResult(sum);
    }

    public String getId() {
        return getClass().getName();
    }
}
