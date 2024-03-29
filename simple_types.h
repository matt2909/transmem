/**
 * Autogenerated by Thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 */
#ifndef simple_TYPES_H
#define simple_TYPES_H

#include <Thrift.h>
#include <reflection_limited_types.h>
#include <protocol/TProtocol.h>
#include <transport/TTransport.h>



namespace simple {

enum MemOp {
  LOAD = 1,
  STORE = 2,
  ABORT = 10,
  BEGIN = 11,
  COMMIT = 12,
  DISABLE = 13,
  ENABLE = 14,
  INFO = 15
};

class Work {
 public:

  static const char* ascii_fingerprint; // = "DE035C7565A0274CBA4FB0CDABB89798";
  static const uint8_t binary_fingerprint[16]; // = {0xDE,0x03,0x5C,0x75,0x65,0xA0,0x27,0x4C,0xBA,0x4F,0xB0,0xCD,0xAB,0xB8,0x97,0x98};

  Work() : cpuid(0), addr(0) {
  }

  virtual ~Work() throw() {}

  int32_t cpuid;
  MemOp type;
  int32_t addr;

  struct __isset {
    __isset() : cpuid(false), type(false), addr(false) {}
    bool cpuid;
    bool type;
    bool addr;
  } __isset;

  bool operator == (const Work & rhs) const
  {
    if (!(cpuid == rhs.cpuid))
      return false;
    if (!(type == rhs.type))
      return false;
    if (!(addr == rhs.addr))
      return false;
    return true;
  }
  bool operator != (const Work &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Work & ) const;

  uint32_t read(facebook::thrift::protocol::TProtocol* iprot);
  uint32_t write(facebook::thrift::protocol::TProtocol* oprot) const;

};

} // namespace

#endif
