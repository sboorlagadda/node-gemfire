# API - Cache

A `cache` consists of data regions, each of which can contain any number of entries. Region entries hold the cached data. Every entry has a key that uniquely identifies it within the region and a value where the data object is stored.

The `Cache` instance allows your process to set general parameters for communication between a cache and other caches in the distributed system, and to create and access any region in the cache.

The cache instance is configured with an XML configuration file via `gemfire.configure()`  or `CacheFactory.create()` and returned by calling `gemfire.getCache()`:

## cache.createRegion(regionName, options)

Adds a region to the GemFire cache. Once the region is created, it will remain in the client for the lifetime of the process. The `regionName` should be a string and the `options` object has a required type property.

 * `options.type`: the type of GemFire region to create. The value should be the string name of one of the GemFire region shortcuts, such as "LOCAL", "PROXY", or "CACHING_PROXY". See the GemFire documentation for [Region Shortcuts](http://gemfire.docs.pivotal.io/latest/userguide/gemfire_nativeclient/client-cache/region-shortcuts.html) and the [apache::geode::client::RegionShortcut C++ enumeration](http://gemfire.docs.pivotal.io/latest/cpp_api/cppdocs/namespacegemfire.html#596bc5edab9d1e7c232e53286b338183) for more details.
 * `options.poolName`: the name of the GemFire pool the region is in. If not specified, a default pool will be used.


Example:

```javascript
cache.getRegion("myRegion") // returns undefined

var myRegion = cache.createRegion("myRegion", {type: "PROXY", poolName: "myPool"});

cache.getRegion("myRegion") // returns the same region as myRegion
```

## cache.executeFunction(functionName, options)

Executes a Java function on a server in the cluster containing the cache. `functionName` is the full Java class name of the function that will be called. Options may be either an array of arguments, or an options object.

 * `options.arguments`: the arguments to be passed to the Java function
 * `options.poolName`: the name of the GemFire pool where the function should be run
 * `options.synchronous`: if true, the function will not run asynchronously.

> **Note**: Unlike region.executeFunction(), `options.filter` is not allowed.

> **Warning:** Due to a workaround for a bug in Gemfire 8.0.0.0, when `options.poolName` is not specified, functions executed by cache.executeFunction() will be executed on exactly one server in the first pool defined in the XML configuration file.

cache.executeFunction returns an EventEmitter which emits the following events:

 * `data`: Emitted once for each result sent by the Java function.
 * `error`: Emitted if the function throws or returns an Exception.
 * `end`: Called after the Java function has finally returned.

> **Warning:** As of GemFire 8.0.0.0, there are some situations where the Java function can throw an uncaught Exception, but the node `error` callback never gets called. This is due to a known bug in how the GemFire 8.0.0.0 Native Client handles exceptions. This bug is only present for cache.executeFunction. region.executeFunction works as expected.

Example:

```javascript
cache.executeFunction("com.example.FunctionName",
    {
      arguments: [1, 2, 3],
      poolName: "myPool"
    }
  )
  .on("error", function(error) { throw error; })
  .on("data", function(result) {
    // ...
  })
  .on("end", function() {
    // ...
  });
```

For more information, please see the [GemFire documentation for Function Execution](http://gemfire.docs.pivotal.io/latest/userguide/developing/function_exec/chapter_overview.html).

## cache.executeFunction(functionName, arguments)

Shorthand for `executeFunction` with an array of arguments. Equivalent to:

```javascript
cache.executeFunction(functionName, { arguments: arguments })
```

## cache.executeQuery(query, [parameters], [options], callback)

Executes an OQL query on the cluster. The callback will be called with an `error` argument and a `response` argument.

 * `query`: a string representing a GemFire OQL query
 * `parameters`: an array of parameters for the query string
 * `options.poolName`: the name of the GemFire pool where the query should be executed

The `response` argument is an object responding to `toArray` and `each`.

 * `response.toArray()`: Return the entire result set as an Array.
 * `response.each(callback)`: Call the callback with a `result` argument, once for each result.

> **Warning:** Due to a workaround for a bug in Gemfire 8.0.0.0, when `options.poolName` is not specified, functions executed by cache.executeQuery() will be executed on exactly one server in the first pool defined in the XML configuration file.

Example:

```javascript
cache.executeQuery("SELECT DISTINCT * FROM /exampleRegion WHERE foo = $1 OR foo = $2", ['bar', 'baz'], {poolName: "myPool"}, function(error, response) {
  if(error) { throw error; }

  var results = response.toArray();
  // allResults could now be this:
  //   [ { foo: 'bar' }, { foo: 'baz' } ]

  // alternately, you could use the `each` iterator:
  response.each(function(result) {
  	// this callback will be called with { foo: 'bar' } then { foo: 'baz' }
  });
}
```

For more information on OQL, see [the documentation](http://gemfire.docs.pivotal.io/latest/userguide/developing/querying_basics/chapter_overview.html).

## cache.getRegion(regionName)

Retrieves a Region from the Cache. An error will be thrown if the region is not present.

Example:

```javascript
var region = cache.getRegion('exampleRegion');
```

### cache.rootRegions()

Retrieves an array of all root Regions from the Cache.

Example:

```javascript
var regions = cache.rootRegions();
// if there are three Regions defined in your cache, regions could now be:
// [firstRegionName, secondRegionName, thirdRegionName]
```
