#include "tm_utils.h"

void foo(void)
{
    printf("Hello world!\n");
}

void mop_print(generic_transaction_t *mop)
{
    const char *op_type, *op_dir;
    
    op_type = SIM_mem_op_is_data(mop) ? "data" : "instr";
    op_dir  = SIM_mem_op_is_read(mop) ? "read" : "write";
    
    printf("%6s: 0x%8x 0x%8x %-5s %2d\n",
            op_type,
            (unsigned int)mop->logical_address,
            (unsigned int)mop->physical_address,
            op_dir,
            mop->size);
}
