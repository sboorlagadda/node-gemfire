package io.pivotal.node_gemfire;

import com.gemstone.gemfire.cache.execute.FunctionAdapter;
import com.gemstone.gemfire.cache.execute.FunctionContext;
import java.math.BigInteger;

public class ReturnNegativeAmbiguousLong extends FunctionAdapter {

    public void execute(FunctionContext fc) {
        BigInteger two = new BigInteger("2");
        long l = 0 - two.pow(53).longValue();
        fc.getResultSender().lastResult(l);
    }

    public String getId() {
        return getClass().getName();
    }
}
