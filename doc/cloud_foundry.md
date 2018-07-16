# Cloud Foundry

Using cloud foundry to deploy your app has many advantages like scaling, deploying etc.    In return all we have todo as developers is parse the environment that gets injected into the process.    By injecting some of the environment details that are bound by the platform make moving an application between environments seamless.

So what does that look like:

```javascript
var gemfire = require('gemfire');
const propertiesFile = 'gemfire.properties';
var cacheFactory = gemfire.createCacheFactory(propertiesFile);
var credentials = JSON.parse(process.env.VCAP_SERVICES)["p-cloudcache"][0].credentials;

// Get the user name and password from vcap services
for (var item in credentials.users) {
    var currUser = credentials.users[item];
    if((currUser.roles.indexOf("developer") > -1)){
        cacheFactory.set("security-username", currUser.username);
        cacheFactory.set("security-password", currUser.password);
    }
}

// Get the locators from the vcap services
for (var item  in credentials.locators) {
    var locator = credentials.locators[item];
    var host = locator.slice(0, locator.indexOf("["));
    var port = locator.slice(locator.indexOf("[") + 1, locator.indexOf("]"));
    cacheFactory.addLocator(host, parseInt(port));
}

cache = cacheFactory.create();
region = cache.createRegion("myRegion", {type: "CACHING_PROXY"});

region.put('foo', { bar: ['baz', 'qux'] }, function(error) {
  region.get('foo', function(error, value) {
    console.log(value); // => { bar: ['baz', 'qux'] }
  });
});
```

In that sample our application username and password and connection details were injected into the process.   This allowed our application to be dynamically instantiated.  How awesome is that!
