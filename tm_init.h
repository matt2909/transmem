#ifndef TM_INIT_H
#define TM_INIT_H

#ifdef __cplusplus
extern "C" {
#endif 
    #include <simics/api.h>
    void init_controller(int num_cpus);
    int memory_operation(generic_transaction_t *mop);
    int memory_observe(generic_transaction_t *mop);
    void begin_transaction();
    void begin_transaction_idx(conf_object_t* obj, lang_void* data);
    //void begin_transaction_pc(logical_address_t pc);
    void commit_transaction();
    void commit_transaction_idx(conf_object_t* obj, lang_void* data);
    void abort_transaction();
    void resume_transaction();
    void abort_transaction_idx(conf_object_t* obj, lang_void* data);
    void disable_interrupts();
    void enable_interrupts();
    void early_release(int var);
    void dump_stats();
    int in_transaction(int cpu);
    int in_exception(int cpu);
    int handling_exception(int cpu, int exception);
    int clearing_exception(int cpu, int exception);
    void undo();
#ifdef __cplusplus    
}
#endif

#endif
