#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "transaction.h"
#include <vector>
#include "thrift_client.h"
#include "simple_types.h"
#include "tm_harness_magic.h"

extern "C" {
#include "simics/api.h"
#include "tm_init.h"
}

class Controller {
public:
    Controller(int num_cpus);
    ~Controller();
    void BeginTransaction();
    void BeginTransactionIdx(int cpu_num);
    void CommitTransaction();
    void CommitTransactionIdx(int cpu_num);
    void AbortTransaction();
    void AbortTransactionIdx(int cpu_num);
    void DisableInterrupts();
    void EnableInterrupts();
    void EarlyRelease(int var);
    int MemoryOperation(generic_transaction_t *mop);
    int MemoryObserve(generic_transaction_t *mop);
    void DumpStats(void);
    bool InTransaction(int cpu);
    bool InException(int cpu);
    int handlingException(int cpu, int exception);
    int clearingException(int cpu, int exception);
private:
    Thrift_Client* tc;
    int cont_num_cpus;
    int tid;
    int* txs;
    std::vector<Transaction *> trans_handler;
};

#endif
