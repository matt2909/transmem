#include "tm_init.h"
#include "tm_global.h"
#include "controller.h"
#include <stdio.h>
#include <cassert>

void init_controller(int num_cpus) 
{
    global_controller = new Controller(num_cpus);
}

void begin_transaction()
{
    assert(global_controller != NULL);
    global_controller->BeginTransaction();
}

/*void begin_transaction_pc(logical_address_t pc)
{
    assert(global_controller != NULL);
    global_controller->BeginTransactionPC(pc);
}*/

void commit_transaction()
{
    assert(global_controller != NULL);
    global_controller->CommitTransaction();
}

void abort_transaction()
{
    assert(global_controller != NULL);
    global_controller->AbortTransaction();
}

void resume_transaction()
{
    assert(global_controller != NULL);
    global_controller->ResumeTransaction();
}

void disable_interrupts()
{
    assert(global_controller != NULL);
    global_controller->DisableInterrupts();
}

void enable_interrupts()
{
    assert(global_controller != NULL);
    global_controller->EnableInterrupts();
}

void early_release(int var)
{
    assert(global_controller != NULL);
    global_controller->EarlyRelease(var);
}

int memory_operation(generic_transaction_t *mop)
{
    assert(global_controller != NULL);
    return global_controller->MemoryOperation(mop);
}

int memory_observe(generic_transaction_t *mop)
{
    assert(global_controller != NULL);
    return global_controller->MemoryObserve(mop);
}

int in_transaction(int cpu)
{
    assert(global_controller != NULL);
    return global_controller->InTransaction(cpu) == true ? 1 : 0;
}

int in_exception(int cpu)
{
    assert(global_controller != NULL);
    return global_controller->InException(cpu) == true ? 1 : 0;
}


int handling_exception(int cpu, int exception)
{
   assert(global_controller != NULL);
   return global_controller->handlingException(cpu, exception);
}

int clearing_exception(int cpu, int exception)
{
   assert(global_controller != NULL);
   return global_controller->clearingException(cpu, exception);
}

void dump_stats()
{
    assert(global_controller != NULL);
    global_controller->DumpStats();
}

void begin_transaction_idx(conf_object_t* obj, lang_void* data) {
    assert(global_controller != NULL);
    global_controller->BeginTransactionIdx(SIM_get_processor_number(obj));
}

void abort_transaction_idx(conf_object_t* obj, lang_void* data) {
    assert(global_controller != NULL);
    global_controller->AbortTransactionIdx(SIM_get_processor_number(obj));
}

void commit_transaction_idx(conf_object_t* obj, lang_void* data) {
    assert(global_controller != NULL);
    global_controller->CommitTransactionIdx(SIM_get_processor_number(obj));
}

void undo() {
    printf("Undo called!\n");
}
