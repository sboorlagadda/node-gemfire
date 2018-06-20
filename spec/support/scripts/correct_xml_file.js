const gemfire = require("../gemfire.js");
gemfire.configure("xml/ExampleClient.xml", "./gfcpp.properties");
const cache = gemfire.getCache();

if(!cache.getRegion('exampleRegion')) {
  throw("Region not found");
}
