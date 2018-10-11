# API - CacheFactory

The CacheFactory allows the connection details to be dynamically provided.   As a convienance the GemFire defaults can be passed into the `gemfire.createCacheFactory(pathToProperties)` then be overridden if needed.

## cacheFactory.addLocator(host, port)

Adds a locator, given its host and port, to this factory.   A locator is the proces where servers register themselves when they come on-line.   This enables GemFire servers and clients to dynamically find each other at runtime.

_Parameters_

* **host**	is the host name or ip address that the locator is listening on.
* **port**	is the port that the locator is listening on.

_Returns_

A reference to this CacheFactory

## cacheFactory.addServer(host, port)

Directly connect to the server, given its host and port, to this factory.


_Parameters_
* **host** is the host name or ip address that the server is listening on.
* **port** is the port that the server is listening on.

_Returns_

A reference to this CacheFactory

## cacheFactory.create()
Once we are done setting any connection properites we use this method to instanciate the Cache.

_Returns_

A reference to the [Cache](cache.md)

## cacheFactory.set(name, value)
Sets a geode property that will be used when creating the Cache.

_Parameters_
* **name** the name of the geode property
* **value** the value of the geode property

_Returns_

A reference to this CacheFactory

## cacheFactory.setFreeConnectionTimeout(connectionTimeout)

Sets the free connection timeout for this pool.

If the pool has a max connections setting, operations will block if all of the connections are in use. The free connection timeout specifies how long those operations will block waiting for a free connection before receiving an AllConnectionsInUseException. If max connections is not set this setting has no effect.

_Parameters_
* **connectionTimeout**	is the connection timeout in milliseconds

_Returns_

A reference to this CacheFactory

## cacheFactory.setIdleTimeout(idleTimeout)

Sets the amount of time a connection can be idle before expiring the connection.

If the pool size is greater than the minimum specified by cacheFactory.setMinConnections(int), connections which have been idle for longer than the idleTimeout will be closed.

_Parameters_

* **idleTimeout**	is the amount of time in milliseconds that an idle connection should live before expiring. -1 indicates that connections should never expire.

_Returns_

A reference to this CacheFactory

## cacheFactory.setLoadConditioningInterval(loadConditioningInterval)

Sets the load conditioning interval for this pool.

This interval controls how frequently the pool will check to see if a connection to a given server should be moved to a different server to improve the load balance.

A value of -1 disables load conditioning

_Parameters_
* **loadConditioningInterval** is the connection lifetime in milliseconds

_Returns_

A reference to this CacheFactory

## cacheFactory.setMaxConnections(maxConnections)

Sets the max number of client to server connections that the pool will create.

If all of the connections are in use, an operation requiring a client to server connection will block until a connection is available.

_Parameters_
* **maxConnections** is the maximum number of connections in the pool. -1 indicates that there is no maximum number of connections

_Returns_

A reference to this CacheFactory

## cacheFactory.setMinConnections(minConnections)

Sets the minimum number of connections to keep available at all times.

When the pool is created, it will create this many connections. If 0 then connections will not be made until an actual operation is done that requires client-to-server communication.

_Parameters_
* **minConnections** is the initial number of connections this pool will create.

_Returns_

A reference to this CacheFactory

## cacheFactory.setPdxIgnoreUnreadFields(ignore)

Control whether pdx ignores fields that were unread during deserialization.

The default is to preserve unread fields be including their data during serialization. But if you configure the cache to ignore unread fields then their data will be lost during serialization.

You should only set this attribute to true if you know this member will only be reading cache data. In this use case you do not need to pay the cost of preserving the unread fields since you will never be reserializing pdx data.

_Parameters_
* **ignore** true if fields not read during pdx deserialization should be ignored; false, the default, if they should be preserved.

_Returns_

A reference to this CacheFactory

## cacheFactory.setPingInterval(pingInterval)

The frequency with which servers must be pinged to verify that they are still alive.

Each server will be sent a ping every pingInterval if there has not been any other communication with the server.

These pings are used by the server to monitor the health of the client. Make sure that the pingInterval is less than the maximum time between pings allowed by the server.

_Parameters_
* **pingInterval** is the amount of time in milliseconds between pings.

_Returns_

A reference to this CacheFactory

## cacheFactory.setPRSingleHopEnabled(enabled)

By default setPRSingleHopEnabled is true
The client is aware of location of partitions on servers hosting Regions.

Using this information, the client routes the client cache operations directly to the server which is hosting the required partition for the cache operation. If setPRSingleHopEnabled is false the client can do an extra hop on servers to go to the required partition for that cache operation. 

If enabled it is advisable to also set `cacheFactory.setMaxConnections(int)` to -1.  This will help prevent unneeded closing of on the connnections in the connection pool as GemFire looks for a open connection to a given server.

_Parameters_
* **name** is boolean whether PR Single Hop optimization is enabled or not.

_Returns_

A reference to this CacheFactory

## cacheFactory.setReadTimeout(timeout)

Sets the number of milliseconds to wait for a response from a server before timing out the operation and trying another server (if any are available).

_Parameters_

* **timeout**	is the number of milliseconds to wait for a response from a server

_Returns_

A reference to this CacheFactory

## cacheFactory.setRetryAttempts(retryAttempts)

Set the number of times to retry a request after timeout/exception.

_Parameters_

* **retryAttempts**	is the number of times to retry a request after timeout/exception. -1 indicates that a request should be tried against every available server before failing.

_Returns_

A reference to this CacheFactory

## cacheFactory.setServerGroup(groupName)

Configures the group which contains all the servers that this pool connects to.

_Parameters_
* **groupName** is the server group that this pool will connect to. If the value is null or "" then the pool connects to all servers.

_Returns_

A reference to this CacheFactory

## cacheFactory.setSocketBufferSize(bufferSize)

Sets the socket buffer size for each connection made in this pool.

Large messages can be received and sent faster when this buffer is larger. Larger buffers also optimize the rate at which servers can send events for client subscriptions.


_Parameters_

* **bufferSize** is the size of the socket buffers used for reading and writing on each connection in this pool.

_Returns_

A reference to this CacheFactory

## cacheFactory.setStatisticInterval(statisticInterval)

The frequency with which the client statistics must be sent to the server.

Doing this allows statistics to monitor clients.

A value of -1 disables the sending of client statistics to the server.

_Parameters_

* **statisticInterval** is the amount of time in milliseconds between sends of client statistics to the server.

_Returns_

A reference to this CacheFactory

## cacheFactory.setSubscriptionAckInterval(ackInterval)

Sets the is the interval in milliseconds to wait before sending acknowledgements to the bridge server for events received from the server subscriptions.

_Parameters_

* **ackInterval** is the number of milliseconds to wait before sending event acknowledgements.

_Returns_

A reference to this CacheFactory

## cacheFactory.setSubscriptionEnabled(enabled)

If set to true then the created pool will have server-to-client subscriptions enabled.

If set to false then all Subscription* attributes are ignored at the time of creation.


_Parameters_

* **enabled** If the subscription sound be enabled for the default pool connection.

_Returns_

A reference to this CacheFactory

## cacheFactory.setSubscriptionMessageTrackingTimeout(messageTrackingTimeout)

Sets the messageTrackingTimeout attribute which is the time-to-live period, in milliseconds, for subscription events the client has received from the server.

It is used to minimize duplicate events. Entries that have not been modified for this amount of time are expired from the list.


_Parameters_

* **messageTrackingTimeout** is the number of milliseconds to set the timeout to.

_Returns_

A reference to this CacheFactory

## cacheFactory.setSubscriptionRedundancy(redundancy)

Sets the redundancy level for this pools server-to-client subscriptions.

If 0 then no redundant copies are kept on the servers. Otherwise an effort is made to maintain the requested number of copies of the server-to-client subscriptions. At most, one copy per server is made up to the requested level.

_Parameters_
* **redundancy** is the number of redundant servers for this client's subscriptions.

_Returns_

A reference to this CacheFactory

## cacheFactory.setThreadLocalConnections(threadLocalConnections)

Sets the thread local connections policy for this pool.

If true then any time a thread goes to use a connection from this pool it will check a thread local cache and see if it already has a connection in it. If so it will use it. If not it will get one from this pool and cache it in the thread local. This gets rid of thread contention for the connections but increases the number of connections the servers see.

If false then connections are returned to the pool as soon as the operation being done with the connection completes. This allows connections to be shared amonst multiple threads keeping the number of connections down.

_Parameters_
* **threadLocalConnections** if true then enable thread local connections.

_Returns_

A reference to this CacheFactory

## cacheFactory.setUpdateLocatorListInterval(updateLocatorListInterval)

The frequency with which client updates the locator list.

To disable this set its value to 0.

_Parameters_
* **updateLocatorListInterval** is the amount of time in milliseconds between checking locator list at locator.

_Returns_

A reference to this CacheFactory
