# API - gemfire

The `gemfire` object is returned by `require("gemfire")` and is the global entry point into node-gemfire.   The `gemfire` object is singleton and there will be only one instance per process.

## gemfire.createCacheFactory() and gemfire.createCacheFactory(propertiesFilePath)

The Cache Factory method allows the developer to programmatically set the details of connection.   This is extremely useful if the environment requires a late binding of some property.

There are two variants to this api - the first just assumes all of the GemFire defaults for any properties.   The second allows the developer to externalize some of the properties in a file.

 For more details on the CacheFactory object returned please refer to the [documentation](cache_factory.md)

 ```javascript
 var gemfire = require('gemfire');
 const propertiesFile = 'gemfire.properties';
 var cacheFactory = gemfire.createCacheFactory(propertiesFile);

cacheFactory.addLocator(host, parseInt(port));
cache = cacheFactory.create();
region = cache.createRegion("myRegion", {type: "CACHING_PROXY"});

 ```
## gemfire.configure(xmlFilePath) and gemfire.configure(xmlFilePath, propertiesFilePath)

 Tells GemFire which declarative cache XML configuration file to use. `xmlFilePath` can be either absolute or relative to the current working directory in the application's environment. Once set, the configuration cannot be changed.

 For more information on cache configuration files, see [the documentation](http://gemfire-native.docs.pivotal.io/latest/geode/cache-init-file/chapter-overview.html).

 Example:

 ```javascript
 var gemfire = require('gemfire');

 gemfire.configure("config/myGemfireConfiguration.xml");
 gemfire.configure("config/anotherGemfireConfiguration.xml"); // throws an error
 ```

## gemfire.connected()

Returns true if the GemFire client is connected to a GemFire distributed system, and false if not.

Example:

```javascript
var gemfire = require('gemfire');

// using the declarative XML method for connecting to the distributed system.
gemfire.configure("config/gemfire.xml");
gemfire.connected(); // returns true

// ... network troubles cause a disconnection ...

gemfire.connected(); // returns false

// ... client automatically reconnects to the GemFire system ...

gemfire.connected(); // returns true
```

### gemfire.gemfireVersion

Returns the version of the GemFire C++ Native Client that has been compiled into node-gemfire.

Example:

```javascript
var gemfire = require('gemfire');
gemfire.gemfireVersion // returns "9.2"
```

### gemfire.getCache()

Returns the cache singleton object. `gemfire.configure()` or the CacheFatory.create() must have been called prior to calling `gemfire.getCache()`.


Example:

```javascript
var gemfire = require('gemfire');

gemfire.getCache(); // throws an error, because gemfire has not been configured yet

gemfire.configure("config/gemfire.xml");

gemfire.getCache(); // returns the cache singleton object

gemfire.getCache(); // returns the same cache singleton object on subsequent calls
```

### gemfire.version

Returns the version of node-gemfire.

Example:

```javascript
var gemfire = require('gemfire');
gemfire.version 
```
