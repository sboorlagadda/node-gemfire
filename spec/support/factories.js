const gemfire = require("./gemfire.js");
gemfire.configure("xml/ExampleClient.xml", "./gfcpp.properties");
exports.getCache = gemfire.getCache;
