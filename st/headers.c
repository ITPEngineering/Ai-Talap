#include "iec_glue.h"           /* for struct dheader */

extern unsigned long long common_ticktime__;
extern void config_init__(void);
extern void config_run__(unsigned long tick);

struct theader __attribute__((section(".theader"))) header = {
    &common_ticktime__,
    config_init__,
    config_run__
};

/* .data segment header with pointers to firmware functions callable
   from IEC program.  No init here.  The header is filled with actual
   pointers to functions in plcprog.c when loading .data of an IEC
   program in RAM */
struct dheader __attribute__((section(".dheader"))) dheader;
