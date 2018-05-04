const childProcess = require('child_process');

const errorMatchers = require("./error_matchers");

function runExternalTest(name, callback) {
  if(!callback) { throw("You must pass a callback");  }

  var filename = "spec/support/scripts/" + name + ".js";

  childProcess.execFile("node", [filename], callback);
}

exports.expectExternalSuccess = function expectExternalSuccess(name, callback){
  jasmine.addMatchers(errorMatchers);

  runExternalTest(name, function(error, stdout, stderr) {
    expect(error).not.toBeError();
    expect(stderr).toEqual('');
    callback(error, stdout);
  });
};

exports.expectExternalFailure = function expectExternalFailure(name, callback, message){
  jasmine.addMatchers(errorMatchers);

  runExternalTest(name, function(error, stdout, stderr) {
    console.log("name " + name + "\n" + 
                "error " + error+ "\n" + 
                "stderr - " + stderr+ "\n" +   
                "message - " + message + "\n" +
                "callback - " + callback + "\n" +
                "index of message -> " + stderr.indexOf(message));

    expect(error).not.toBeNull();
    expect(stderr.indexOf(message) >= 0).toBe(true);
    console.log("message - " + message + " *******");
    callback(error, stdout, stderr);
  });
};
