package io.pivotal.node_gemfire;

import org.apache.geode.cache.Cache;
import org.apache.geode.cache.CacheFactory;
import org.apache.geode.cache.execute.FunctionAdapter;
import org.apache.geode.cache.execute.FunctionContext;
import org.apache.geode.pdx.PdxInstanceFactory;

public class ReturnObjectArrayTriggersError extends FunctionAdapter {

    public void execute(FunctionContext fc) {
        Object[] objectArray = new Object[] {
                "000000000000000000000000",
                "000000000000000000000000",
                "000000000000000000000000",
                "000000000000000000000000",
                "000000000000000000000000",
                "000000000000000000000000",
                "000000000000000000000000",
                "000000000000000000000000",
                "000000000000000000000000",
                "000000000000000000000000",
                "000000000000000000000000"};

        Cache cache = CacheFactory.getAnyInstance();
        PdxInstanceFactory pdxInstanceFactory = cache.createPdxInstanceFactory("JSON object");
        pdxInstanceFactory.writeObjectArray("someArray", objectArray);

        fc.getResultSender().lastResult(pdxInstanceFactory.create());
    }

    public String getId() {
        return getClass().getName();
    }

}
