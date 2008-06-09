#ifndef MEMORYTRACE_H
#define MEMORYTRACE_H

#include <map>
#include <iostream>
extern "C" {
#include "simics/api.h"
#include "simics/arch/x86.h"
}

using namespace std;

#define LINE_BYTES 16

class MemOp {
public:
   MemOp(processor_t* cpu, physical_address_t addr, int, uint32);
   ~MemOp();
   physical_address_t getAddress(void);
   uint32 getData(physical_address_t addr, int);
   void setData(physical_address_t addr, int, uint32);
   int getSize(void);
private:
  physical_address_t mAddress;
  unsigned int mData;
  int mSize;	
  void setChunk(int, int, uint32);
  uint32 getChunk(int, int);
};

class MemoryTrace {
public:
    MemoryTrace(processor_t *);
    ~MemoryTrace();
    void clear();
    void commit_writes();
    bool has_read(physical_address_t addr, int size);
    int getWriteBufferSize();
    physical_address_t getBufferedWrite(int index);
    void OperateMemoryRead(generic_transaction_t *);
    void OperateMemoryWrite(generic_transaction_t *);
    void ObserveMemoryRead(generic_transaction_t *);
    void ObserveMemoryWrite(generic_transaction_t *);
    void earlyRelease(int address);
    uinteger_t getHighData(generic_transaction_t *);
    uinteger_t getLowData(generic_transaction_t *);
private:
    processor_t *cpu;
    unsigned int observes;
    unsigned int operates;
    unsigned int unique_writes;
    unsigned int read_from_writes;
    map<physical_address_t, char> read_set;
    map<physical_address_t, MemOp*> write_set;
};


#endif
