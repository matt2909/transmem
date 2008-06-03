#!/usr/local/bin/thrift -cpp -java -r

/**
 * The first thing to know about are types. The available types in Thrift are:
 *
 *  bool        Boolean, one byte
 *  byte        Signed byte
 *  i16         Signed 16-bit integer
 *  i32         Signed 32-bit integer
 *  i64         Signed 64-bit integer
 *  double      64-bit floating point value
 *  string      String
 *  map<t1,t2>  Map from one type to another
 *  list<t1>    Ordered list of one type
 *  set<t1>     Set of unique elements of one type
 *
 * Did you also notice that Thrift supports C style comments?
 */

// Just in case you were wondering... yes. We support simple C comments too.

/**
 * Thrift files can namespace, package, or prefix their output in various
 * target languages.
 */
namespace cpp simple
namespace java simple

/**
 * You can define enums, which are just 32 bit integers. Values are optional
 * and start at 1 if not supplied, C style again.
 */
enum MemOp {
  LOAD = 1,
  STORE = 2,
  ABORT = 0xA,
  BEGIN = 0xB,
  COMMIT = 0xC,
  DISABLE = 0xD,
  ENABLE = 0xE,
  INFO = 0xF
}

/**
 * Structs are the basic complex data structures. They are comprised of fields
 * which each have an integer identifier, a type, a symbolic name, and an
 * optional default value.
 */
struct Work {
  1: i32 cpuid, 
  2: MemOp type,
  3: i32 addr
}

/**
 * Ahh, now onto the cool part, defining a service. Services just need a name
 * and can optionally inherit from another service using the extends keyword.
 */
service MemoryHierarchy {

  void step(),
  /** 
   *  lets have a simple generic method for now that recieves all events
   *  we can always optimize this at a later point.
   */
   void operate(1:Work w)
}
