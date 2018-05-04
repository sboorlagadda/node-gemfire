package io.pivotal.node_gemfire;

import org.apache.geode.cache.Region;
import org.apache.geode.cache.execute.FunctionAdapter;
import org.apache.geode.cache.execute.FunctionContext;
import org.apache.geode.cache.execute.RegionFunctionContext;

import java.util.List;
import java.util.Map;
import java.util.Set;

public class Put extends FunctionAdapter {

    public void execute(FunctionContext fc) {
        RegionFunctionContext regionFunctionContext = (RegionFunctionContext) fc;
        Region<Object, Object> region = regionFunctionContext.getDataSet();
        List arguments = (List) regionFunctionContext.getArguments();
        region.put(arguments.get(0), arguments.get(1));

        fc.getResultSender().lastResult(true);
    }

    public String getId() {
        return getClass().getName();
    }
}
