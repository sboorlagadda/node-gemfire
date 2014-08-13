var childProcess = require('child_process');

describe("gemfire.Region", function() {
  var region, cache;

  beforeEach(function() {
    var gemfire = require("../gemfire.js");
    cache = new gemfire.Cache();
    region = cache.getRegion("exampleRegion");
    region.clear();
  });

  afterEach(function(done) {
    setTimeout(done, 0);
  });

  describe(".put/.get", function() {
    it("returns undefined for unknown keys", function() {
      expect(region.get('foo')).toBeUndefined();
    });

    it("stores and retrieves values in the correct region", function() {
      var region1 = cache.getRegion("exampleRegion");
      var region2 = cache.getRegion("anotherRegion");

      region1.clear();
      region2.clear();

      region1.put("foo", "bar");
      expect(region1.get("foo")).toEqual("bar");
      expect(region2.get("foo")).toBeUndefined();

      region2.put("foo", 123);
      expect(region1.get("foo")).toEqual("bar");
      expect(region2.get("foo")).toEqual(123);
    });

    it("stores and retrieves strings", function() {
      expect(region.put("foo", "bar")).toEqual("bar");
      expect(region.get("foo")).toEqual("bar");

      expect(region.put("empty string", "")).toEqual("");
      expect(region.get("empty string")).toEqual("");

      var wideString =  "\u0123\u4567\u89AB\uCDEF\uabcd\uef4A"
      expect(region.put("wide string", wideString)).toEqual(wideString);
      expect(region.get("wide string")).toEqual(wideString);
    });

    it("stores and retrieves booleans", function() {
      expect(region.put("true", true)).toEqual(true);
      expect(region.get("true")).toEqual(true);

      expect(region.put("false", false)).toEqual(false);
      expect(region.get("false")).toEqual(false);
    });

    it("stores and retrieves dates", function() {
      var now = new Date();
      expect(region.put('now', now)).toEqual(now);
      expect(region.get('now')).toEqual(now);
    });

    it("stores and retrieves arrays", function() {
      var emptyArray = [];
      expect(region.put('empty array', emptyArray)).toEqual(emptyArray);
      expect(region.get('empty array')).toEqual(emptyArray);

      var complexArray = ['a string', 2, true, null, false, { an: 'object' }, new Date(), ['another array', true]];
      expect(region.put('empty array', complexArray)).toEqual(complexArray);
      expect(region.get('empty array')).toEqual(complexArray);

      var sparseArray = [];
      sparseArray[10] = 'an element';
      expect(region.put('sparse array', sparseArray)).toEqual(sparseArray);
      expect(region.get('sparse array')).toEqual(sparseArray);

      var deepArray = [[[[[[[[[[[[[[[[[[["Not too deep"]]]]]]]]]]]]]]]]]]];
      expect(region.put('deep array', deepArray)).toEqual(deepArray);
      expect(region.get('deep array')).toEqual(deepArray);
    });

    it("stores and retrieves numbers", function() {
      expect(region.put("foo", 1.23)).toEqual(1.23);
      expect(region.get("foo")).toEqual(1.23);

      expect(region.put("one", 1)).toEqual(1);
      expect(region.get("one")).toEqual(1);

      expect(region.put("zero", 0.0)).toEqual(0.0);
      expect(region.get("zero")).toEqual(0.0);

      expect(region.put("MAX_VALUE", Number.MAX_VALUE)).toEqual(Number.MAX_VALUE);
      expect(region.get("MAX_VALUE")).toEqual(Number.MAX_VALUE);

      expect(region.put("MIN_VALUE", Number.MIN_VALUE)).toEqual(Number.MIN_VALUE);
      expect(region.get("MIN_VALUE")).toEqual(Number.MIN_VALUE);

      expect(region.put("NaN", Number.NaN)).toBeNaN();
      expect(region.get("NaN")).toBeNaN();

      expect(region.put("NaN", NaN)).toBeNaN();
      expect(region.get("NaN")).toBeNaN();

      expect(region.put("NEGATIVE_INFINITY", Number.NEGATIVE_INFINITY)).toEqual(Number.NEGATIVE_INFINITY);
      expect(region.get("NEGATIVE_INFINITY")).toEqual(Number.NEGATIVE_INFINITY);

      expect(region.put("POSITIVE_INFINITY", Number.POSITIVE_INFINITY)).toEqual(Number.POSITIVE_INFINITY);
      expect(region.get("POSITIVE_INFINITY")).toEqual(Number.POSITIVE_INFINITY);
    });

    it("stores and retrieves null", function() {
      expect(region.put("foo", null)).toBeNull();
      expect(region.get("foo")).toBeNull();
    });

    it("can use the empty string as a key", function() {
      expect(region.put('', 'value')).toEqual('value');
      expect(region.get('')).toEqual('value');
    });

    it("stores and retrieves objects", function() {
      expect(region.put('object', { baz: 'quuux' })).toEqual({ baz: 'quuux' });
      expect(region.get('object')).toEqual({ baz: 'quuux' });

      expect(region.put('empty object', {})).toEqual({});
      expect(region.get('empty object')).toEqual({});

      expect(region.put('nested object', { foo: { bar: 'baz' } })).toEqual( { foo: { bar: 'baz' } } );
      expect(region.get('nested object')).toEqual( { foo: { bar: 'baz' } } );

      expect(region.put('object containing array', { foo: [] })).toEqual( { foo: [] });
      expect(region.get('object containing array')).toEqual( { foo: [] });

      var object = require("./fixtures/stress_test.json");
      expect(region.put('stress test', object)).toEqual(object);
      expect(region.get('stress test')).toEqual(object);
    });
  });

  describe(".clear", function(){
    it("removes all keys", function(){
      region.put('key', 'value');
      expect(region.get('key')).toBeDefined();
      region.clear();
      expect(region.get('key')).toBeUndefined();
    });

    it("only removes keys for the region, not for other regions", function() {
      var region1 = cache.getRegion("exampleRegion");
      var region2 = cache.getRegion("anotherRegion");

      region1.clear();
      region2.clear();

      region1.put('key', 'value');
      region2.put('key', 'value');

      expect(region1.get('key')).toBeDefined();
      expect(region2.get('key')).toBeDefined();

      region1.clear();

      expect(region1.get('key')).not.toBeDefined();
      expect(region2.get('key')).toBeDefined();
    });
  });

  describe(".onPut", function() {
    describe("for puts triggered locally", function() {
      afterEach(function() {
        region.unregisterAllKeys();
      });

      var key = Date.now().toString();

      var callback1 = jasmine.createSpy();
      var callback2 = jasmine.createSpy();

      beforeEach(function(done) {
        region.registerAllKeys();
        region.onPut(callback1);
        region.onPut(callback2);
        setTimeout(done, 0);
      });

      beforeEach(function() {
        expect(callback1).not.toHaveBeenCalled();
        expect(callback2).not.toHaveBeenCalled();
      });

      beforeEach(function(done) {
        region.put(key, "bar");
        setTimeout(done, 0);
      });

      beforeEach(function() {
        expect(callback1).toHaveBeenCalledWith(key, "bar");
        expect(callback2).toHaveBeenCalledWith(key, "bar");
      });

      beforeEach(function(done) {
        region.put(key, "baz");
        setTimeout(done, 0);
      });

      it("fires the callback function when a put occurs", function() {
        expect(callback1).toHaveBeenCalledWith(key, "baz");
        expect(callback2).toHaveBeenCalledWith(key, "baz");
      });
    });

    describe("for puts triggered externally", function() {
      var callback;

      function triggerExternalPut(done){
        childProcess.execFile("node", ["spec/support/external_put.js"], function(error, stdout, stderr) {
          if(error) {
            console.error(stderr);
            expect(error).toBeNull();
            done();
          }
        });

        setTimeout(done, 500);
      }

      function setUpCallback(done){
        callback = jasmine.createSpy("callback").andCallFake(function(key, value){
          done();
        });

        region.onPut(callback);
      }

      describe("when interest in the key has been registered", function() {
        beforeEach(function(done) {
          region.registerAllKeys();
          setUpCallback(done);
          triggerExternalPut(done);
        });

        it("fires the callback function", function() {
          expect(callback).toHaveBeenCalledWith("async", "test");
        });
      });

      describe("when interest in the key has not been registered", function() {
        beforeEach(function(done) {
          region.unregisterAllKeys();
          setUpCallback(done);
          triggerExternalPut(done);
        });

        it("does not call the callback function", function() {
          expect(callback).not.toHaveBeenCalled();
        });
      });
    });
  });
});