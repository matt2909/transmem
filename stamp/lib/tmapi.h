#ifndef TM_API_H_
#define TM_API_H_
/* use to allow c++ files to import it */
#ifdef __cplusplus
extern "C" {
#endif

#define MAGIC(n) do {                                \
  asm volatile ("movl %0, %%eax" : : "g" (n) : "eax");  \
  asm volatile ("xchg %bx,%bx");                        \
} while (0)

#define RELEASE(var) do {                           \
  asm volatile ("movl %0, %%edx" : : "g" (var) : "edx"); \
  asm volatile ("movl %0, %%eax" : : "g" (TX_RELEASE) : "eax"); \
  asm volatile ("xchg %bx,%bx");                       \
} while (0)


#define MAGIC_BREAKPOINT MAGIC(0)
    
#define TX_ABORT     0xA
#define TX_BEGIN     0xB
#define TX_COMMIT    0xC
#define TX_DISABLE   0xD
#define TX_ENABLE    0xE
#define TX_INFO      0xF
#define SIM_ON       0xAA
#define SIM_OFF      0xBB
#define TX_RELEASE   0xCC

#ifdef __cplusplus
}
#endif

#endif /*TM_HARNESS_MAGIC_H_*/
