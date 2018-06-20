const gemfire = require('../gemfire.js');
gemfire.configure("spec/support/scripts/no_regions.xml", "./gfcpp.properties");
const cache = gemfire.getCache();
cache.executeFunction("io.pivotal.node_gemfire.TestFunction");
