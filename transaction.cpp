#include "transaction.h"
#include "x86_flags.h"
using namespace simple;

Transaction::Transaction(unsigned int cpu_num, Thrift_Client* tc)
{
    
    tm_cpu_num = cpu_num;
    mTc = tc;
    cpu = SIM_get_processor(tm_cpu_num);
    clearStats();
    memory_trace = new MemoryTrace(cpu);
    nesting = 0;
    mPauseDepth = 0;
    mHandlingException = false;
    mLastLoad = 0;

    /* Checkpointing state for the x86-p4 floating point unit */
    fpu_status_att = (attr_value_t*)malloc(sizeof(attr_value_t));
    fpu_status_att->kind = Sim_Val_Integer;
    fpu_status_att->u.integer = 0;
    
    fpu_control_att = (attr_value_t*)malloc(sizeof(attr_value_t));
    fpu_control_att->kind = Sim_Val_Integer;
    fpu_control_att->u.integer = 0;

    fpu_tag_att = (attr_value_t*)malloc(sizeof(attr_value_t));
    fpu_tag_att->kind = Sim_Val_Integer;
    fpu_tag_att->u.integer = 0;
   
    fpu_registers_att = (attr_value_t*)malloc(sizeof(attr_value_t));
    fpu_registers_att->kind = Sim_Val_List;
    fpu_registers_att->u.list.size = 8; //8 80-bit fp registers
    fpu_registers_att->u.list.vector = (attr_value_t*)malloc(8 * sizeof(attr_value_t));
    for(int i = 0; i < 8; i++) {
       fpu_registers_att->u.list.vector[i].kind = Sim_Val_Data;
       fpu_registers_att->u.list.vector[i].u.data.size = 11; //10 * 8 bytes, plus empty/valid byte
       fpu_registers_att->u.list.vector[i].u.data.data = new uint8[11];
   }

   /* Checkpointing state for the x86-p4 sse unit */
   /* Checkpointing state for the x86-p4 mmx registers */
    
     
}

Transaction::~Transaction() 
{
    //clean up any storage for this transaction
    delete memory_trace;    
    registers.clear();
    fpu_registers.clear();
}

bool Transaction::runningTransaction() {
    return (tm_status == TRANS_ON);
}

int Transaction::nestingLevel() {
    return nesting;
}
/*processor_t* Transaction::getCPU() {
    return cpu;
}*/

/*void Transaction::SetBeginPC(logical_address_t begin_pc)
{
    pc = begin_pc;
}*/

void Transaction::BeginTransaction(int process_num) 
{
    nesting++;
    assert(nesting != 0); //test for overflow
    
    if(nesting == 1) 
    {
        // grab the program counter
        pc = SIM_get_program_counter(cpu);
        tm_process_num = process_num;
        tm_status = TRANS_ON;
        //clear sets
        //disable interrupts
        cout << "[" << tm_cpu_num << "] Begin  transaction (" << process_num << ")" << endl;
        mTc->operate(tm_cpu_num, BEGIN, (int)SIM_cycle_count(cpu)); 
        //cout << SIM_get_program_counter(SIM_current_processor()) << endl;
        //cout << "Begin started at " << SIM_get_program_counter(cpu) << endl;
        checkpointRegisters();
        //disableInterrupts();
        clearStats();
    }
    else {
       cout << "nesting > 1? [cpu:" << tm_cpu_num << "] ==> " << nesting << " resetting to 1 " << endl;
       nesting = 1;
    }
    //SIM_clear_exception();
}

void Transaction::CommitTransaction()
{
    if(tm_status == TRANS_PAUSE) { //commit triggered at start of exception nesting
       tm_status = TRANS_OFF;      //set transactions off we don't want to do anything
	clearStats();
        memory_trace->commit_writes();
        memory_trace->clear();
        registers.clear();
        fpu_registers.clear();
        pc = 0;
        tm_process_num = -1;
        return;
     }
     else if(tm_status == TRANS_OFF) {
        nesting--;
        if(nesting != 0) 
        {
           cout << "[assert] nesting depth [cpu:" << tm_cpu_num << "] ==> " << nesting << endl;
           nesting = 0;
        }
        assert(((int)nesting) == 0);
        if(nesting == 0) {
           assert(mHandlingException); //if we've just handled a exception wake all
           mHandlingException = false;

           memory_trace->clear();
           registers.clear();
           fpu_registers.clear();
           pc = 0;
           tm_process_num = -1;
        }
     }
     else { //default case inside well-behaved transactions!
        nesting--;
        if(nesting != 0) {
           cout << "[normal] nesting depth [cpu:" << tm_cpu_num << "] ==> " << nesting << endl;
           nesting = 0;
        }
        assert(((int)nesting) == 0);
        assert(!mHandlingException);
        if(nesting == 0)
        {
           tm_status = TRANS_OFF;
           enableInterrupts();
           cout << "[" << tm_cpu_num << "] Commit transaction (" << tm_process_num << 
                   ")" << endl;
           clearStats();
           memory_trace->commit_writes();
           memory_trace->clear();
           registers.clear();
           fpu_registers.clear();
           pc = 0;
           tm_process_num = -1;
        }
    }
    mTc->operate(tm_cpu_num, COMMIT, (int)SIM_cycle_count(cpu));
}

void Transaction::AbortTransaction()
{
    cout << "[" << tm_cpu_num << "] Abort  transaction (" << tm_process_num << 
            ") depth " << nesting << endl;
    mTc->operate(tm_cpu_num, ABORT, (int)SIM_cycle_count(cpu));
    nesting = 0;
    memory_trace->clear();
    restoreRegisters();
    registers.clear();
    fpu_registers.clear();
    clearStats();
    tm_status = TRANS_OFF;
    enableInterrupts();
}

void Transaction::clearStats()
{
    memory_ops = 0;
    memory_obs = 0;
    memory_loads = 0;
    memory_stores = 0;
    memory_loads_obs = 0;
    memory_stores_obs = 0;
}

void Transaction::AbortReset()
{
   assert(0);
    /*if(tm_status == TRANS_ABORT)
    {
        tm_status = TRANS_ON;
    }*/
}

bool Transaction::CheckForReadConflict(physical_address_t addr, int size) {
    return memory_trace->has_read(addr, size);
}

bool Transaction::CheckForReadConflict(physical_address_t addr) {
    return CheckForReadConflict(addr, 4);
}

int Transaction::getWriteBufferSize() 
{
    return memory_trace->getWriteBufferSize();
}

physical_address_t Transaction::getBufferedWrite(int index) 
{
    return memory_trace->getBufferedWrite(index);
}

int Transaction::MemoryOperation(generic_transaction_t *mop)
{
    if(tm_status == TRANS_ON) {
        switch(mop->type)
        {
            case Sim_Trans_Load: 
                memory_trace->OperateMemoryRead(mop); 
                memory_ops++;
                memory_loads++;
                return Sim_Trans_Load;
            case Sim_Trans_Store:
                memory_trace->OperateMemoryWrite(mop);
                memory_ops++;   
                memory_stores++;
                return Sim_Trans_Store;
	    default: return -1;
        }
    }
    return -1;
}

int Transaction::MemoryObserve(generic_transaction_t *mop)
{
     if(tm_status == TRANS_ON) {
        switch(mop->type)
        {
            case Sim_Trans_Load: 
                memory_trace->ObserveMemoryRead(mop); 
                memory_obs++;
                memory_loads_obs++;
                return Sim_Trans_Load;
            case Sim_Trans_Store:
                memory_trace->ObserveMemoryWrite(mop); 
                memory_obs++;
                memory_stores_obs++;
                return Sim_Trans_Store;
            default: return -1;
        }
    }
    return -1;
}

void Transaction::clear_write_set()
{
    //clear the transactions write set
}

void Transaction::clear_read_set()
{
    //clear the transactions read set
}

/* disable interrupts in an x86 proc */
void Transaction::disableInterrupts() 
{
    int eflag_reg_number = SIM_get_register_number(cpu, "eflags");
    uinteger_t eflag = SIM_read_register(cpu, eflag_reg_number);
    // clear bit 9
    //uinteger_t new_eflag = (eflag & (~0x200));
    uinteger_t new_eflag = (eflag & ~X86_EFLAGS_IF) | X86_EFLAGS_AC; 
    SIM_write_register(cpu, eflag_reg_number, new_eflag);
    
}

/* enable interrupts in an x86 cpu */
void Transaction::enableInterrupts() 
{
    
    int eflag_reg_number = SIM_get_register_number(cpu, "eflags");
    uinteger_t eflag = SIM_read_register(cpu, eflag_reg_number);
    // set bit 9
    //uinteger_t new_eflag = (eflag | 0x200);
    uinteger_t new_eflag = (eflag | X86_EFLAGS_IF) & (~X86_EFLAGS_AC);
    SIM_write_register(cpu, eflag_reg_number, new_eflag);
}

void Transaction::earlyRelease(int var)
{
   memory_trace->earlyRelease(var);
}

void Transaction::checkpointRegisters()
{
    //get the data type containing all registers
    attr_value_t all_registers = SIM_get_all_registers(cpu);
    //convert to a list, strictly check that it is of type list first
    attr_list_t all_reg_list = all_registers.u.list;
    
    for(int i = 0; i < all_reg_list.size; i++) {
        registers.push_back(SIM_read_register(cpu, 
                all_reg_list.vector[i].u.integer));
    }
    
    //read floating point registers
    attr_list_t fpu_regs = SIM_get_attribute(cpu, "fpu_regs").u.list;
    for(int i = 0; i < fpu_regs.size; i++) {
    //    cout << "[" << tm_cpu_num << "] fpureg[" << i << "] ";
        //uint8 data[fpu_regs.vector[i].u.data.size];
        for(int j = 0; j < fpu_regs.vector[i].u.data.size; j++) {
           fpu_registers_att->u.list.vector[i].u.data.data[j] = fpu_regs.vector[i].u.data.data[j];
    //       cout << (int)data[j] << " ";
        }
    //    cout << endl;
    }
    
    fpu_status_att->u.integer =  SIM_get_attribute(cpu, "fpu_status").u.integer;
    fpu_control_att->u.integer = SIM_get_attribute(cpu, "fpu_control").u.integer;
    fpu_tag_att->u.integer = SIM_get_attribute(cpu, "fpu_tag").u.integer;
    //cout << "fpu_status " << fpu_status << endl;
    //cout << "fpu_control " << fpu_control << endl;
    //cout << "fpu_tag " << fpu_tag << endl;

    /*
    fpu_commit_last_instr = SIM_get_attribute(cpu, "fpu_commit_last_instr").u.integer;
    fpu_last_instr_pointer0 = SIM_get_attribute(cpu, "fpu_last_instr_pointer0").u.integer;
    fpu_last_instr_pointer1 = SIM_get_attribute(cpu, "fpu_last_instr_pointer1").u.integer;
    fpu_last_instr_selector0 = SIM_get_attribute(cpu, "fpu_last_instr_selector0").u.integer;
    fpu_last_instr_selector1 = SIM_get_attribute(cpu, "fpu_last_instr_selector1").u.integer;
    fpu_last_opcode0 = SIM_get_attribute(cpu, "fpu_last_opcode0").u.integer;
    fpu_last_opcode1 = SIM_get_attribute(cpu, "fpu_last_opcode1").u.integer;
    fpu_last_operand_pointer0 = SIM_get_attribute(cpu, "fpu_last_operand_pointer0").u.integer;
    fpu_last_operand_pointer1 = SIM_get_attribute(cpu, "fpu_last_operand_pointer1").u.integer;
    fpu_last_operand_selector0 = SIM_get_attribute(cpu, "fpu_last_operand_selector0").u.integer;
    fpu_last_operand_selector1 = SIM_get_attribute(cpu, "fpu_last_operand_selector1").u.integer;*/
}

void Transaction::restoreRegisters()
{
    //printf("Register list size == %d\n", (int)registers.size());
    for(int i = 0; i < (int)registers.size(); i++)
    {
        //if(i < 26) {
           SIM_write_register(cpu, i, registers[i]);
        //}
    }

   attr_list_t fpu_regs = SIM_get_attribute(cpu, "fpu_regs").u.list;
   SIM_set_attribute(cpu, "fpu_regs", fpu_registers_att); 
/*    for(int i = 0; i < (int)fpu_registers.size(); i++)
    {
       cout << "[" << tm_cpu_num << "] fpureg[" << i << "] ";
       for(int j = 0; j < fpu_regs.vector[i].u.data.size; j++) {
          cout << (int)fpu_regs.vector[i].u.data.data[j] << " ";
          fpu_regs.vector[i].u.data.data[j] = fpu_registers[i][j];
          
       }
       cout << endl;
    }
*/    
    //cout << "fpu_status  " << SIM_get_attribute(cpu, "fpu_status").u.integer << endl;
    SIM_set_attribute(cpu, "fpu_status", fpu_status_att);
    //cout << "fpu_statuss " << SIM_get_attribute(cpu, "fpu_status").u.integer << endl;
    //cout << "fpu_control  " << SIM_get_attribute(cpu, "fpu_control").u.integer << endl;
    SIM_set_attribute(cpu, "fpu_control", fpu_control_att);
    //cout << "fpu_controls " << SIM_get_attribute(cpu, "fpu_control").u.integer << endl;
    //cout << "fpu_tag  " << SIM_get_attribute(cpu, "fpu_tag").u.integer << endl;
    SIM_set_attribute(cpu, "fpu_tag", fpu_tag_att);
    //cout << "fpu_tags " << SIM_get_attribute(cpu, "fpu_tag").u.integer << endl;

    /*SIM_get_attribute(cpu, "fpu_commit_last_instr").u.integer = 0; //fpu_commit_last_instr;
    SIM_get_attribute(cpu, "fpu_last_instr_pointer0").u.integer = 0; //fpu_last_instr_pointer0;
    SIM_get_attribute(cpu, "fpu_last_instr_pointer1").u.integer = 0; //fpu_last_instr_pointer1;
    SIM_get_attribute(cpu, "fpu_last_instr_selector0").u.integer = 0; //fpu_last_instr_selector0;
    SIM_get_attribute(cpu, "fpu_last_instr_selector1").u.integer = 0; //fpu_last_instr_selector1;
    SIM_get_attribute(cpu, "fpu_last_opcode0").u.integer = 0; //fpu_last_opcode0;
    SIM_get_attribute(cpu, "fpu_last_opcode1").u.integer = 0; //fpu_last_opcode1;
    SIM_get_attribute(cpu, "fpu_last_operand_pointer0").u.integer = 0; //fpu_last_operand_pointer0;
    SIM_get_attribute(cpu, "fpu_last_operand_pointer1").u.integer = 0; //fpu_last_operand_pointer1;
    SIM_get_attribute(cpu, "fpu_last_operand_selector0").u.integer = 0; //fpu_last_operand_selector0;
    SIM_get_attribute(cpu, "fpu_last_operand_selector1").u.integer = 0; //fpu_last_operand_selector1;*/

    SIM_set_program_counter(cpu, pc);
}


int Transaction::handlingException(int exception)
{
   mHandlingException = true;
   mPauseDepth++;
   if(mPauseDepth == 1) {
      tm_status = TRANS_PAUSE;
      enableInterrupts();
   }
   return mPauseDepth;
}

int Transaction::clearingException(int exception)
{
   mPauseDepth--;
   if(mPauseDepth == 0) 
   {
      //tm_status = TRANS_ON;
      //disableInterrupts();
      //AbortTransaction();
   }
   return mPauseDepth;
}

bool Transaction::inException() 
{
   return (tm_status == TRANS_PAUSE || mHandlingException);
}


bool Transaction::lastLoad(physical_address_t addr) {
   if(addr == mLastLoad) {
      mLastLoad = -1;
      return true;
   }
   else {
      mLastLoad = addr;
      return false;
   }
}
