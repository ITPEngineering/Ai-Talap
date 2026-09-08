#include "plcprog_c_glue.h"     /* for struct dheader */

extern void run();
extern void init();

struct plcprog_c_theader __attribute__((section(".theader"))) header = {
    .magic = PLCPROG_C_MAGIC,
    .objsz = 0, /* size of __data -- unused in plcprog_c */
    .init = init,
    .run = run,
};

/* .data segment header with pointers to firmware functions callable
   from IEC program.  No init here; the header is filled at program
   start by plcprog_c_glue_apply */
struct plcprog_c_dheader __attribute__((section(".dheader"))) dheader;
