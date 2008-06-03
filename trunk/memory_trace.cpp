#include "memory_trace.h"
#include <cassert>
MemoryTrace::MemoryTrace(processor_t *parents_cpu) 
{
    cpu = parents_cpu;
    read_from_writes = 0;
    unique_writes = 0;
    observes = 0;
    operates = 0;
}

MemoryTrace::~MemoryTrace()
{
    //delete read_set;
    //delete write_set;
}

void MemoryTrace::OperateMemoryWrite(generic_transaction_t *mop) 
{
    x86_memory_transaction_t* xmop = (x86_memory_transaction_t*)mop;
    if(xmop->access_type == X86_Vanilla) {
       operates++;
       physical_address_t addr = mop->physical_address; 
       int size = mop->size;
       switch(size) {
           case 8:
               if(write_set.find((addr + 4) & ~3) == write_set.end())
               {
                   unique_writes++;
                   MemOp* memop1 = new MemOp(cpu, (addr + 4), 4, ((SIM_get_mem_op_value_cpu(mop) >> 32) & 0xffffffff));
                   MemOp* memop2 = new MemOp(cpu, addr, 4, (SIM_get_mem_op_value_cpu(mop) & 0xffffffff));
                   write_set[(addr + 4) & ~3] = memop1;
                   write_set[addr & ~3]       = memop2;
                   mop->ignore = 1;
               }
               else { 
                   if(write_set.find(addr & ~3) == write_set.end()) {
                       MemOp* memop = new MemOp(cpu, addr, 4, (SIM_get_mem_op_value_cpu(mop) & 0xffffffff));
                       write_set[addr & ~3] = memop;
                   }
                   write_set[(addr + 4) & ~3]->setData((addr + 4), 4, ((SIM_get_mem_op_value_cpu(mop) >> 32) & 0xffffffff));
                   write_set[addr & ~3]->setData(addr, 4, (SIM_get_mem_op_value_cpu(mop) & 0xffffffff));
                   mop->ignore = 1;
               }
               break;
           case 4: case 2: case 1:
		if(write_set.find(addr & ~3) == write_set.end())
                {
                    unique_writes++;
                    uinteger_t value = (size == 4) ? SIM_get_mem_op_value_cpu(mop) : SIM_get_mem_op_value_le(mop);
                    if(size < 4) { //implicitly we must read the value
                    	if(read_set.find(addr & ~3) == read_set.end()) {
                            read_set[addr & ~3] = '0';
                            //assert(0);
                        }
                    }
                    MemOp* memop = new MemOp(cpu, addr, size, value);
                    write_set[addr & ~3] = memop; 
                    mop->ignore = 1;
                }
                else { 
		    uinteger_t value = (size == 4) ? SIM_get_mem_op_value_cpu(mop) : SIM_get_mem_op_value_le(mop);
                    write_set[addr & ~3]->setData(addr, size, value);
                    mop->ignore = 1;
                }
                break;
            default: cout << "write size == " << size << endl; assert(0); break;
       }
       //cout << "write seen [" << addr << " cpuval " << SIM_get_mem_op_value_cpu(mop) 
       //<< " memval " << SIM_read_phys_memory(SIM_current_processor(), addr, 4) << endl;
       if(mop->size > 4) {
          //-cout << "mop write size == " << mop->size << endl;
	  //assert(mop->size <= 4);
       }       
    }
    else {
      //cout << "access of type " << xmop->access_type << endl;
    }
}

void MemoryTrace::OperateMemoryRead(generic_transaction_t *mop) 
{
   x86_memory_transaction_t* xmop = (x86_memory_transaction_t*)mop;
   if(xmop->access_type == X86_Vanilla) {
      operates++;
      if(mop->size > 4) {
        //cout << "mop read size == " << mop->size << " on processor " << SIM_get_processor_number(cpu) << endl;
      }
      mop->inquiry = 1;
      mop->speculative = 1;
   }
   else {
      //cout << "access of type " << xmop->access_type << endl;
   }
}


void MemoryTrace::ObserveMemoryWrite(generic_transaction_t *mop)
{
    /* this method shouldn't get called. Transactional writes are 
       buffered until committal at which point the commit routine 
       dumps all the writes into memory 
    */
    assert(0);
}

void MemoryTrace::ObserveMemoryRead(generic_transaction_t *mop)
{
    x86_memory_transaction_t* xmop = (x86_memory_transaction_t*)mop;
    if(xmop->access_type == X86_Vanilla) {
       //assert(mop->size <= 4);
       observes++;
       x86_memory_transaction_t* xmop = (x86_memory_transaction_t*)mop;
       assert(xmop->mode == Sim_CPU_Mode_User);
    
       /****************************************************************************
        * ADD READ TO THE READ SET:
        * - double word reads (8 bytes) are added as two seperate 32-bit entries
        ****************************************************************************/
       physical_address_t addr = (mop->physical_address & ~3);
       if(read_set.find(addr) == read_set.end())
       {
          read_set[addr] = '0';
       }
       //if its a double size read, add both to the read set (I think)
       if(mop->size == 8 && read_set.find(addr+4) == read_set.end()) 
       {
          read_set[addr+4] = '0';
          //cout << "type ==> " << SIM_get_mem_op_type_name(mop->type) << endl;
       }
    
       /****************************************************************************
        * CHECK IF READ IS IN WRITE SET:
        * - if in write set hijack the operation and use that value instead of the
        *   value present in main memory.
        * - otherwise get from memory: this is safe as a committing transaction 
        *   will abort a transaction reading from an updated value.
        ****************************************************************************/
       if(write_set.find(addr) != write_set.end()) {
          //cout << "reading from write" << endl;
          read_from_writes++;
          /*if((int)mop->size != write_set[addr]->getSize()) {
	     cout << "Mop->size " << (int)mop->size << " writeSet->size " << write_set[addr]->getSize() << endl;
             cout << "Memory read " << SIM_read_phys_memory(cpu, addr, mop->size) << endl;
             cout << "Stored read " << write_set[addr]->getData(mop->physical_address, mop->size) << endl;
	     cout << "Mem[ws-size] " << SIM_read_phys_memory(cpu, addr, write_set[addr]->getSize()) << endl;
             //assert(0);
          }*/
          if(mop->size == 8) {
               /* if available grab the high data from the write_set */
               assert(write_set.find(addr + 4) != write_set.end());
               uint32 high_data = write_set[addr + 4]->getData(addr + 4, 4);
	       uint32 low_data  = write_set[addr]->getData(addr, 4);
               uinteger_t data = ((uinteger_t)high_data << 32) | (0xffffffff & low_data);
               SIM_set_mem_op_value_cpu(mop, data);
          }
          else if(mop->size == 4) {
               SIM_set_mem_op_value_cpu(mop, write_set[addr]->getData(mop->physical_address, mop->size));
          }
          else if(mop->size < 4) {
             SIM_set_mem_op_value_le(mop, write_set[addr]->getData(mop->physical_address, mop->size));
          }
          else {
             cout << "reading of size " << mop->size << endl;
             assert(0);
          }
       }
    }
}

void MemoryTrace::commit_writes()
{
    map<physical_address_t, MemOp*>::const_iterator iter;
    for (iter=write_set.begin(); iter != write_set.end(); ++iter) {
        //cout << "commiting write " << iter->second << " " << iter->first << endl;
        physical_address_t addr = iter->second->getAddress();
        int size = iter->second->getSize();
        uint32 data = iter->second->getData(addr, size);
        SIM_write_phys_memory(cpu, addr, data, size);
    }
}


bool MemoryTrace::has_read(physical_address_t addr, int size)
{
    
    if(read_set.find(addr & ~3) != read_set.end()) {
        return true;
    }
    if(size > 4) {
        if(read_set.find((addr & ~3) + 4) != read_set.end()) {
            return true;
        }
    }
    return false;
}

void MemoryTrace::clear()
{
    //cout << "unique_reads " << read_set.size() << endl;
    //cout << "unique_writes " << unique_writes << endl;
    //cout << "read_from_writes " << read_from_writes << endl;
    //cout << "observes " << observes << endl;
    //cout << "operates " << operates << endl;
    read_from_writes = 0;
    unique_writes = 0;
    observes = 0;
    operates = 0;

    /* delete all of objects created in the readset */
    read_set.clear();
    
    /* delete all of objects created in the writeset */
    map<physical_address_t, MemOp*>::const_iterator iter;
    for (iter=write_set.begin(); iter != write_set.end(); ++iter) {
       delete iter->second;
    }
    write_set.clear();
}

int MemoryTrace::getWriteBufferSize() 
{
    //cout << "[" << cpu << "] write buffer size ==> " << write_set.size() << endl;
    return write_set.size();
}

physical_address_t MemoryTrace::getBufferedWrite(int index) 
{
    int position = 0;
    map<physical_address_t, MemOp*>::const_iterator iter;
    for (iter=write_set.begin(); iter != write_set.end(); ++iter) {
        if(position == index) {
            return iter->second->getAddress();
        }
        else 
            position++;
    }
    return -1;
}

/** removes lines from a 32-byte cache */
void MemoryTrace::earlyRelease(int address)
{
    //int rs_size = read_set.size();
    //int found = 0;

    for(int i = 0; i < 32; i += 4) {
        physical_address_t remAddress = SIM_logical_to_physical(cpu, Sim_DI_Data, logical_address_t(address + i));

        //if(read_set.find(remAddress) != read_set.end()) found++;
            
        read_set.erase(remAddress);
    }
    //cout << "read_set reduced by " << (rs_size - read_set.size()) << "(" << read_set.size() << "," << rs_size << ") [" << found << "]" << endl;
}


MemOp::MemOp(processor_t* cpu, physical_address_t address, int size, uint32 data) {
    mAddress = address & ~3;
    mSize = 4;
    int offset = address - mAddress;
    /* initialize to global memory values */
    mData = SIM_read_phys_memory(cpu, mAddress, 4);
    
    /* place write into appropriate chunk */
    setChunk(offset, size, data);
}

MemOp::~MemOp() {
}

uint32 MemOp::getData(physical_address_t address, int size) {
   /* check address bounds */
    assert(address >= mAddress);
    assert(address < (mAddress + 4));
    assert(size <= 4);

    /* alignment if half-word */
    if(size == 2)
	assert(address <= (mAddress + 2));
    
    int offset = address - mAddress;
    return getChunk(offset, size);
}

int MemOp::getSize(void) {
  return mSize;
}

physical_address_t MemOp::getAddress(void) {
  return mAddress;
}


void MemOp::setData(physical_address_t address, int size, uint32 data)
{
    /* check address bounds */
    assert(address >= mAddress);
    assert(address < (mAddress + 4));
    assert(size <= 4);

    /* alignment if half-word */
    if(size == 2)
	assert(address <= (mAddress + 2));

    /* having checked bounds and offsets set the chunk */
    int offset = address - mAddress;
    setChunk(offset, size, data);
}

void MemOp::setChunk(int offset, int size, uint32 data)
{
    /* based on x86 little endianess */
    switch(size) {
	case 1: 
         /* overwrite one of the bytes */
	 switch(offset) {
             case 0: mData = (0xFFFFFF00 & mData) | (0xFF & data);         break;
             case 1: mData = (0xFFFF00FF & mData) | ((0xFF & data) << 8);  break;
	     case 2: mData = (0xFF00FFFF & mData) | ((0xFF & data) << 16); break;
             case 3: mData = (0x00FFFFFF & mData) | ((0xFF & data) << 24); break;
             default: assert(0); break;
	 }
         break;
     case 2:
         /* overwrite half word */
         switch(offset) {
             case 0: mData = (0xFFFF0000 & mData) | (0xFFFF & data);         break;
             case 1: mData = (0xFF0000FF & mData) | ((0xFFFF & data) << 8);  break;
             case 2: mData = (0x0000FFFF & mData) | ((0xFFFF & data) << 16); break;
             default: assert(0); break;
         }
         break;
     case 4: 
         /* overwrite the data completely */
	 mData = data;
         break;
     default:
	assert(0);
        break;
    }
}

uint32 MemOp::getChunk(int offset, int size)
{
    /* based on x86 little endianess */
    switch(size) {
        case 1: 
        /* read one of the bytes */
	switch(offset) {
            case 0: return (0x000000FF & mData);
            case 1: return (0x000000FF & (mData >> 8));
	    case 2: return (0x000000FF & (mData >> 16));
            case 3: return (0x000000FF & (mData >> 24));
            default: assert(0); break;
	}
        break;
    case 2:
        /* read half word */
        switch(offset) {
            case 0: return (0x0000FFFF & mData);
            case 1: return (0x0000FFFF & (mData >> 8));
            case 2: return (0x0000FFFF & (mData >> 16));
            default: assert(0); break;
        }
        break;
    case 4: 
        /* read the data completely */
        return mData;
        break;
    default:
	assert(0);
        break;
    }
}
