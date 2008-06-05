/*
  transaction_module.c - Skeleton code to base new device modules on

  Copyright 1998-2007 Virtutech AB
  
  The contents herein are Source Code which are a subset of Licensed
  Software pursuant to the terms of the Virtutech Simics Software
  License Agreement (the "Agreement"), and are being distributed under
  the Agreement.  You should have received a copy of the Agreement with
  this Licensed Software; if not, please contact Virtutech for a copy
  of the Agreement prior to using this Licensed Software.
  
  By using this Source Code, you agree to be bound by all of the terms
  of the Agreement, and use of this Source Code is subject to the terms
  the Agreement.
  
  This Source Code and any derivatives thereof are provided on an "as
  is" basis.  Virtutech makes no warranties with respect to the Source
  Code or any derivatives thereof and disclaims all implied warranties,
  including, without limitation, warranties of merchantability and
  fitness for a particular purpose and non-infringement.

*/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <simics/api.h>
#include <simics/alloc.h>
#include <simics/utils.h>
#include <simics/arch/x86.h>

#include "tm_harness_magic.h"
#include "tm_utils.h"
#include "tm_init.h"

/* USER-TODO: Set the name of the device class */
#define DEVICE_NAME "transmem"
#define TRANS_OFF 0
#define TRANS_ON  1

timing_model_interface_t *timing_iface;
snoop_memory_interface_t *snoop_iface;

typedef struct {
    conf_object_t obj;
    timing_model_interface_t *timing_iface;
    snoop_memory_interface_t *snoop_iface;
} transaction_module_t;

/*
 * Initialize the transaction_module
 */
static conf_object_t *
transaction_module_new_instance(parse_object_t *parse_obj)
{
    init_controller(SIM_number_processors());
    transaction_module_t *transmod = MM_ZALLOC(1, transaction_module_t);
    SIM_object_constructor((conf_object_t *) transmod, parse_obj);
    transmod->timing_iface = timing_iface;
    transmod->snoop_iface = snoop_iface;
    return (conf_object_t *) transmod;
}

static cycles_t
transaction_module_operate(conf_object_t *obj, conf_object_t *space,
                           map_list_t *map, generic_transaction_t *mop)
{
    /* do the operate stuff here */
    if(SIM_mem_op_is_from_cpu(mop)) 
    {
        return memory_operation(mop);
    }
    //SIM_clear_exception();
    return 0;
}


static cycles_t
transaction_module_observe(conf_object_t *obj, conf_object_t *space,
                           map_list_t *map, generic_transaction_t *mop)
{
    /* do the observe stuff here */
    if(SIM_mem_op_is_from_cpu(mop)) 
    {
        memory_observe(mop);
    }
    //SIM_clear_exception();
    return 0;
}

static void magic_called(void *callback_data, conf_object_t *obj,
			   integer_t parameter)
{
    processor_t* cpu  = SIM_current_processor();
    int reg_number = SIM_get_register_number(cpu, "eax");
    int eax = SIM_read_register(cpu, reg_number);
    int edx_reg_number = SIM_get_register_number(cpu, "edx");
    int edx = SIM_read_register(cpu, edx_reg_number);
    
    switch(eax) {
        case TX_BEGIN:
	    begin_transaction();
            break;
        case TX_COMMIT:
            commit_transaction();
            break;
        case TX_ABORT:
            abort_transaction();
            break;
        case TX_DISABLE:
            disable_interrupts();
            break;
        case TX_ENABLE:
            enable_interrupts();
            break;
        case TX_INFO:
            printf("Simulation cycles ==> %ld\n", (long)SIM_cycle_count(cpu));
            dump_stats();
            break;
        case TX_RELEASE:
	    early_release(edx);
	    break;
    }  
    SIM_clear_exception();
}

static void core_context_change(void *callback_data, conf_object_t *obj,
			   integer_t parameter)
{
   int cpu_num = SIM_get_processor_number(SIM_current_processor());
   if(in_transaction(cpu_num)) {
      printf("Core context change on proc %d\n", cpu_num);
   }
   SIM_clear_exception();
}

int exception_depth = 0;

static void core_exception(void *callback_data, conf_object_t *obj,
			   integer_t parameter)
{
   
   int cpu_num = SIM_get_processor_number(SIM_current_processor());
   if(in_transaction(cpu_num) || in_exception(cpu_num)) {
      printf("****Core Exception START (%d) on proc %d\n%s\n", (int)parameter, 
	     cpu_num, SIM_get_exception_name(SIM_current_processor(), parameter));
      int depth = handling_exception(cpu_num, (int)parameter);
      printf("Depth ==> %d\n", depth);
      if(depth == 1) {
         int i = 0;
         for(i = 0; i < SIM_number_processors(); i++) {
            if(i != cpu_num) {
               if(in_transaction(i)) {
		  SIM_stall_cycle(SIM_get_processor(i), 0xffffffff);
                  //printf("=============================> Disabling a core while in a transaction!\n");
               }
               //SIM_disable_processor(SIM_get_processor(i));
               //SIM_stall_cycle(SIM_get_processor(i), 0x8ffffff);
            }
         }   
      }
   }
   SIM_clear_exception();
}

static void core_exception_return(void *callback_data, conf_object_t *obj,
			   integer_t parameter)
{
   int cpu_num = SIM_get_processor_number(SIM_current_processor());
   if(in_transaction(cpu_num) || in_exception(cpu_num)) {
      printf("****Core Exception ENDS on proc %d\n", cpu_num);
      int depth = clearing_exception(cpu_num, (int)parameter);
      printf("Depth ==> %d\n", depth);
      if(depth == 0) {
         int i = 0;
         for(i = 0; i < SIM_number_processors(); i++) {
            if(i != cpu_num & in_transaction(i))
                SIM_stall_cycle(SIM_get_processor(i), 0xffffffff);
         }
      }
   }
   SIM_clear_exception();
}

static void core_mode_switch(void *callback_data, conf_object_t *obj,
			   integer_t parameter)
{
   int cpu_num = SIM_get_processor_number(SIM_current_processor());
   if(in_transaction(cpu_num) || in_exception(cpu_num)) {
      printf("****Core Mode Switch (%d) on proc %d\n", (int)parameter, cpu_num);
   }
   SIM_clear_exception();
}


/* init_local() is called once when the device module is loaded into Simics */
void
init_local(void)
{
        class_data_t class_data;
        conf_class_t *conf_class;
	
        hap_handle_t h1;
        hap_handle_t h2;
	hap_handle_t h3;
	hap_handle_t h4;
	hap_handle_t h5;

        /* Register the empty device class. */
        memset(&class_data, 0, sizeof(class_data_t));
        class_data.new_instance = transaction_module_new_instance;
	class_data.description =
		"This device is and experimental TM module called "
		DEVICE_NAME " - currently not stable!.";
        conf_class = SIM_register_class(DEVICE_NAME, &class_data);

        /* initialize and register the timing model */
        timing_iface = MM_ZALLOC(1, timing_model_interface_t);
        timing_iface->operate = transaction_module_operate;
        SIM_register_interface(conf_class, "timing_model", timing_iface);
        
        snoop_iface = MM_ZALLOC(1, snoop_memory_interface_t);
        snoop_iface->operate = transaction_module_observe;
        SIM_register_interface(conf_class, "snoop_memory", snoop_iface);
        
        
        
        /* Add a call back to capture core magic instruction events */
	h1 = SIM_hap_add_callback("Core_Magic_Instruction", 
				  (obj_hap_func_t)magic_called,
				  NULL);

	h2 = SIM_hap_add_callback("Core_Context_Change",
				  (obj_hap_func_t)core_context_change,
				  NULL);

	h3 = SIM_hap_add_callback("Core_Exception", 
				  (obj_hap_func_t)core_exception,
				  NULL);

	h4 = SIM_hap_add_callback("Core_Exception_Return", 
				  (obj_hap_func_t)core_exception_return,
				  NULL);

        h5 = SIM_hap_add_callback("Core_Mode_Switch",
				  (obj_hap_func_t)core_mode_switch,
				  NULL);

}
