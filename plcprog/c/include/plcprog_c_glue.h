#pragma once

#include <stdint.h>

enum {PLCPROG_C_MAGIC = 0x576ae72d};

/* .text segment of a plcprog/c prog starts with this structure.  The
   layout exactly matches struct theader from iec_glue.h to avoid
   unnecessary branching in plcprog.c */
struct __attribute__ ((aligned(4))) plcprog_c_theader {
    int magic; /* make sure the program and the loader versions match */
    uint32_t objsz;               /* size of __data */
    void (*init)(void);
    void (*run)(void);
};

/* .data segment of a plcprog/c prog starts with this structure.
   Going through the dispatch function on every call is less efficient
   than direct function pointers in iec_glue.h struct dheader, but
   more maintainable */
struct __attribute__ ((aligned(8))) plcprog_c_dheader {
    void *(*dispatch)(char *method_name);
};


/* in plcprog_c_glue.c */
void plcprog_c_glue_apply (void *data_start);
