#include "controller.h"

using namespace std;
using namespace simple;
Controller::Controller(int num_cpus)
{
    tid = 0;
    cont_num_cpus = num_cpus;
    cout << "TM controller:: cores == " << cont_num_cpus << "\n";
    //trans_handler.setSize(cont_num_cpus);
    tc = new Thrift_Client();
    cout << "TM controller created a thrift client" << endl;
    txs = new int[cont_num_cpus];
    for(int i = 0; i < cont_num_cpus; i++) 
    {
       trans_handler.push_back(new Transaction(i, tc));
       txs[i] = 0;
    }
    
 
    
}

Controller::~Controller()
{
    for(int i = 0; i < cont_num_cpus; i++) 
        delete trans_handler[i];
}

/*void Controller::BeginTransactionPC(logical_address_t pc)
{
    int cpu_num = SIM_get_processor_number(SIM_current_processor());
    trans_handler[cpu_num]->SetBeginPC(pc);
}*/

void Controller::BeginTransaction()
{

    int cpu_num = SIM_get_processor_number(SIM_current_processor());
    //txs[cpu_num]++;
    //cout << "[" << cpu_num << "] cont-begin  == " << txs[cpu_num] << endl;
    //assert(txs[cpu_num] == 1);
    if(trans_handler[cpu_num]->nestingLevel() == 0) {
      trans_handler[cpu_num]->BeginTransaction(tid); 
      tid++;
      //SIM_stacked_post(SIM_get_processor(cpu_num), begin_transaction_idx, NULL);
      //tc->operate(cpu_num, BEGIN, (int)SIM_cycle_count(SIM_current_processor())); 
    }
}

void Controller::BeginTransactionIdx(int cpu_num)
{
    trans_handler[cpu_num]->BeginTransaction(tid++);
    SIM_clear_exception();
}

void Controller::CommitTransaction()
{
    int cpu_num = SIM_get_processor_number(SIM_current_processor());
    int bufferSize = trans_handler[cpu_num]->getWriteBufferSize();

    for(int i = 0; i < cont_num_cpus; i++) 
    {
       if(i != cpu_num && trans_handler[i]->runningTransaction()) {
          for(int w = 0; w < bufferSize; w++)
          {
             physical_address_t addr = trans_handler[cpu_num]->getBufferedWrite(w);
             if(trans_handler[i]->CheckForReadConflict(addr & ~3))
             {
                 //trans_handler[i]->AbortTransaction();
                 cout << "[" << cpu_num << "] aborts [" << i << "]" << endl;
                 SIM_stacked_post(SIM_get_processor(i), abort_transaction_idx, NULL);
                 break;
             }
          }
       }
    }

/*    for(int w = 0; w < bufferSize; w++)
    {
        physical_address_t addr = trans_handler[cpu_num]->getBufferedWrite(w);
        for(int i = 0; i < cont_num_cpus; i++) 
        {
            if(i != cpu_num && trans_handler[i]->runningTransaction()) {
                if(trans_handler[i]->CheckForReadConflict(addr & ~3))
                {
                    //trans_handler[i]->AbortTransaction();
                    SIM_stacked_post(SIM_get_processor(i), abort_transaction_idx, NULL);
                    
	       }
            }
        }
    }
*/
    SIM_stacked_post(SIM_current_processor(), commit_transaction_idx, NULL);
    //trans_handler[cpu_num]->CommitTransaction();
}

void Controller::CommitTransactionIdx(int cpu_num)
{
   trans_handler[cpu_num]->CommitTransaction();
   if(SIM_get_pending_exception()) {
      SIM_last_error();
   }
   SIM_clear_exception();
}

void Controller::AbortTransaction()
{
    int cpu = SIM_get_processor_number(SIM_current_processor());
    cout << "[" << cpu << "] self-aborting " << endl;
    SIM_stacked_post(SIM_current_processor(), abort_transaction_idx, NULL);
    //assert(0);
    //int cpu_num = SIM_get_processor_number(SIM_current_processor());
    //--//txs[cpu_num]--;
    //--//cout << "[" << cpu_num << "] cont-abort  == " << txs[cpu_num] << endl;
    //--//assert(txs[cpu_num] == 0);
    //trans_handler[cpu_num]->AbortTransaction();
    //--//tc->operate(cpu_num, ABORT, (int)SIM_cycle_count(SIM_current_processor()));
}

void Controller::AbortTransactionIdx(int cpu_num)
{
    if(trans_handler[cpu_num]->runningTranscation()) {
        trans_handler[cpu_num]->AbortTransaction();
        if(SIM_get_pending_exception()) {
            SIM_last_error();
        }
        SIM_clear_exception();
    }
}

void Controller::DisableInterrupts()
{
    int cpu_num = SIM_get_processor_number(SIM_current_processor());
    trans_handler[cpu_num]->disableInterrupts();
}

void Controller::EnableInterrupts()
{
    int cpu_num = SIM_get_processor_number(SIM_current_processor());
    trans_handler[cpu_num]->enableInterrupts();
}

void Controller::EarlyRelease(int var)
{
    int cpu_num = SIM_get_processor_number(SIM_current_processor());
    trans_handler[cpu_num]->earlyRelease(var);
}


int Controller::MemoryOperation(generic_transaction_t *mop) 
{
    if(SIM_mem_op_is_from_cpu(mop)) {
        int cpu_num = SIM_get_processor_number(SIM_current_processor());
        if(trans_handler[cpu_num]->runningTransaction()) 
        {
            int res = trans_handler[cpu_num]->MemoryOperation(mop);
            if(res == Sim_Trans_Load) {
		//tc->operate(cpu_num, LOAD, (int)mop->physical_address);
                if(!trans_handler[cpu_num]->lastLoad(mop->physical_address))
                   return 10;
            }
            else if(res == Sim_Trans_Store) {
               //tc->operate(cpu_num, STORE, (int)mop->physical_address);
            }
        }
    }
    return 0;
}

int Controller::MemoryObserve(generic_transaction_t *mop) 
{
    int cpu_num = SIM_get_processor_number(SIM_current_processor());
    if(trans_handler[cpu_num]->runningTransaction()) 
    {
        int res = trans_handler[cpu_num]->MemoryObserve(mop);
	if(res == Sim_Trans_Load) {
	   //tc->operate(cpu_num, LOAD, (int)mop->physical_address);
           //return 10;
	}
        else if(res == Sim_Trans_Store) {
           //tc->operate(cpu_num, STORE, (int)mop->physical_address);
        }
    }
    else { //not transaction, need to compare to other transactions
       for(int i = 0; i < cont_num_cpus; i++) { 
          if(i != cpu_num && trans_handler[i]->runningTransaction() && mop->type == Sim_Trans_Store) {
             int addr = mop->physical_address;
             int size = mop->size;
             if(trans_handler[i]->CheckForReadConflict(addr, size))
             {
                cout << "Non-transactional write on core [" << cpu_num << "] kills core [" << i << "]!" << endl;
                //trans_handler[i]->AbortTransaction();
                SIM_stacked_post(SIM_get_processor(i), abort_transaction_idx, NULL);
		//tc->operate(i, ABORT, (int)SIM_cycle_count(SIM_current_processor()));
             }
          }
       }
    }
    return 0;
}

bool Controller::InTransaction(int cpu)
{
   return trans_handler[cpu]->runningTransaction();
}

bool Controller::InException(int cpu)
{
   return trans_handler[cpu]->inException();
}

int Controller::handlingException(int cpu, int exception)
{
   int depth = trans_handler[cpu]->handlingException(exception);
   //if(depth == 1) 
   //CommitTransaction();
   return depth;
}

int Controller::clearingException(int cpu, int exception)
{
   return trans_handler[cpu]->clearingException(exception);
}


void Controller::DumpStats(void)
{
    int cpu_num = SIM_get_processor_number(SIM_current_processor());
    tc->operate(cpu_num, TX_INFO, 0);
} 

