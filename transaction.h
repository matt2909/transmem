#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "memory_trace.h"
#include <iostream>
#include <vector>
#include <cassert>
#include "thrift_client.h"
#include "simple_types.h"

extern "C" {
    #include <simics/api.h>
    #include <simics/arch/x86.h>
    #include <simics/core/obsolete.h>
}

using namespace std;

#define TRANS_OFF 0
#define TRANS_ON  1
#define TRANS_ABORT 2
#define TRANS_PAUSE 3

class Transaction {
public:
    Transaction(unsigned int cpu_num, Thrift_Client* tc);
    ~Transaction();
    bool runningTransaction();
    bool inException();
    //void SetBeginPC(logical_address_t begin_pc);
    void BeginTransaction(int process_num);
    void CommitTransaction();
    void AbortTransaction();
    void ResumeTransaction();
    bool CheckForReadConflict(physical_address_t addr);
    bool CheckForReadConflict(physical_address_t addr, int size);
    void AbortReset();
    int MemoryOperation(generic_transaction_t *mop);
    int MemoryObserve(generic_transaction_t *mop);
    int  getWriteBufferSize();
    physical_address_t  getBufferedWrite(int index);
    void disableInterrupts();
    void enableInterrupts();
    void earlyRelease(int var);
    int handlingException(int exception);
    int clearingException(int exception);
    int nestingLevel();
    bool lastLoad(physical_address_t addr);
    void clearTransaction();
    //processor_t* getCPU();

private:
    int mStatus;
    unsigned int mCpuNum;
    processor_t *cpu;
    unsigned int tm_process_num;
    unsigned int tm_nesting;
    unsigned int memory_ops;
    unsigned int memory_loads;
    unsigned int memory_stores;
    unsigned int memory_obs;
    unsigned int memory_loads_obs;
    unsigned int memory_stores_obs;
    unsigned int nesting;
    MemoryTrace *memory_trace;
    void clearStats();
    void clear_write_set();
    void clear_read_set();
    bool mHandlingException;
    void checkpointRegisters();
    void restoreRegisters();
    vector<long> registers;
    vector<uint8*> fpu_registers;
    uint32 fpu_status;
    attr_value_t* fpu_status_att;
    attr_value_t* fpu_control_att;
    attr_value_t* fpu_tag_att;
    attr_value_t* fpu_registers_att;
    uint32 fpu_control;
    uint32 fpu_tag;
    uint32 fpu_commit_last_instr;
    uint32 fpu_last_instr_pointer0;
    uint32 fpu_last_instr_pointer1;
    uint32 fpu_last_instr_selector0;
    uint32 fpu_last_instr_selector1;
    uint32 fpu_last_opcode0;
    uint32 fpu_last_opcode1;
    uint32 fpu_last_operand_pointer0;
    uint32 fpu_last_operand_pointer1;
    uint32 fpu_last_operand_selector0;
    uint32 fpu_last_operand_selector1;
    
    int pc;
    int mPauseDepth;
    physical_address_t mLastLoad;
    Thrift_Client* mTc;
};


#endif
