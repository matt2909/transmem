/*
  transmem.c - simics module registration code for 
  the transactional memory module.
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

/* Declare this module to have the name transmem in simics */
#define DEVICE_NAME "transmem"

/* Local pointers to the timing and snoop interfaces       */
timing_model_interface_t *timing_iface;
snoop_memory_interface_t *snoop_iface;

/* A struct used to hold the transmem module */
typedef struct {
    conf_object_t obj;
    timing_model_interface_t *timing_iface;
    snoop_memory_interface_t *snoop_iface;
} transmem_t;

/* simics instantiation of new configuration object for transmem */
static conf_object_t *
transmem_new_instance(parse_object_t *parse_obj)
{
    init_controller(SIM_number_processors());
    transmem_t *transmem = MM_ZALLOC(1, transmem_t);
    SIM_object_constructor((conf_object_t *) transmem, parse_obj);
    transmem->timing_iface = timing_iface;
    transmem->snoop_iface = snoop_iface;
    return (conf_object_t *) transmem;
}

/* operate: used by the timing interface the returned cycles 
 * delays the memory operation. 
 */
static cycles_t
transaction_module_operate(conf_object_t *obj, conf_object_t *space,
                           map_list_t *map, generic_transaction_t *mop)
{
    if(SIM_mem_op_is_from_cpu(mop)) 
    {
        return memory_operation(mop);
    }
    
    return 0;
}


/* observe can be used to snoop returned memory values */
static cycles_t
transaction_module_observe(conf_object_t *obj, conf_object_t *space,
                           map_list_t *map, generic_transaction_t *mop)
{
    if(SIM_mem_op_is_from_cpu(mop)) 
    {
        memory_observe(mop);
    }
    return 0;
}

/* magic handler called when the Magic hap callback is called */
static void magic_called(void *callback_data, conf_object_t *obj,
			   integer_t parameter)
{
    /* inquire which processor made the magic call */
    processor_t* cpu  = SIM_current_processor();
    /* get the value of eax */
    int reg_number = SIM_get_register_number(cpu, "eax");
    int eax = SIM_read_register(cpu, reg_number);
    /* get the value of edx, used for early release */
    int edx_reg_number = SIM_get_register_number(cpu, "edx");
    int edx = SIM_read_register(cpu, edx_reg_number);
    
    switch(eax) {
        case TX_BEGIN:   begin_transaction();  break;
        case TX_COMMIT:  commit_transaction(); break;
        case TX_ABORT:   abort_transaction();  break;
        case TX_DISABLE: disable_interrupts(); break;
        case TX_ENABLE:  enable_interrupts();  break;
        case TX_INFO:    dump_stats();         break;
        case TX_RELEASE: early_release(edx);   break;
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
      //int depth = handling_exception(cpu_num, (int)parameter);
      //printf("[%2d] Exception start (%2d) -- %s\n", cpu_num, depth, SIM_get_exception_name(SIM_current_processor(), parameter));
      
      /*if(depth == 1) 
      {
           //SIM_stacked_post(SIM_current_processor(), abort_transaction, NULL);
	   printf("setting undo\n");
           SIM_stacked_post(SIM_current_processor(), undo, NULL);
           SIM_breakpoint(SIM_current_processor(), 
				Sim_Break_Physical, 
				Sim_Access_Execute, 
				SIM_get_program_counter(SIM_current_processor()),
				4,
				Sim_Breakpoint_Temporary);
      			
      }
      printf("Depth ==> %d\n", depth);
      */
   }
   SIM_clear_exception();
}

static void core_exception_return(void *callback_data, conf_object_t *obj,
			   integer_t parameter)
{
   int cpu_num = SIM_get_processor_number(SIM_current_processor());
   if(in_transaction(cpu_num) || in_exception(cpu_num)) {
      //printf("****Core Exception ENDS on proc %d\n", cpu_num);
      int depth = clearing_exception(cpu_num, (int)parameter);
      //printf("[%2d] Exception ends  (%2d)\n", cpu_num, depth);
      //printf("Depth ==> %d\n", depth);
      if(depth == 0) {
          //printf("Transaction restarted by exception completion\n");
	  //abort_transaction();
          SIM_stacked_post(SIM_current_processor(), resume_transaction, NULL);
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

static void core_breakpoint(void *callback_data, conf_object_t *obj,
			   integer_t parameter)
{

   int cpu_num = SIM_get_processor_number(SIM_current_processor());
   printf("breakpoint hit %d\n", cpu_num);
   SIM_stacked_post(SIM_current_processor(), abort_transaction, NULL);
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
	hap_handle_t h6;

        /* Register the empty device class. */
        memset(&class_data, 0, sizeof(class_data_t));
        class_data.new_instance = transmem_new_instance;
	class_data.description =
		"This device is an experimental TM module called "
		DEVICE_NAME " - currently pre alpha!.";
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
        h6 = SIM_hap_add_callback("Core_Breakpoint", 
				  (obj_hap_func_t)core_breakpoint,
				  NULL);

}
