#ifndef IEC_GLUE_H
#define IEC_GLUE_H

#include <inttypes.h>
#include <iec_types.h>          /* matiec/lib/C/iec_types.h */

enum {IEC_GLUE_MAGIC = 0xbe9efaf1};

/* .text segment starts with this structure containing pointers to IEC
   program functions and data needed by plcprog.c to run the IEC code */
struct __attribute__ ((aligned(4))) theader {
    int magic; /* make sure the program and the loader versions match */
    size_t objsz;               /* size of __data */
    void (*init)(void *__data, int use_retain);
    void (*run)(void *__data);
};

/* The `dheader' structure is placed at the beginning of .data segment
   and initialized at load time with pointers to functions that IEC
   runtime needs */
struct __attribute__ ((aligned(8))) dheader {
    IEC_TIMESPEC (*get_current_time) (void);
    double (*get_value) (char *name);
    void (*set_value) (char *name, double val);
    void (*set_initvalue) (char *name, double val);
    void (*scalar_check) (char *name);
    void (*binvar_alloc) (char *name, size_t size);
    void *(*binvar_open) (char *name, int offset, int size, void **state);
    void (*binvar_close) (void *state, int offset, int size);
    int (*printf) (const char *, ...);
};


extern struct dheader dheader;  /* in st/mk-headers.pl */

/* macros used in accessor.h definitions */
#define BROKER_SCALAR_CHECK(NAME)\
    dheader.scalar_check(NAME)
#define BROKER_BINVAR_ALLOC(NAME, SIZE)\
    dheader.binvar_alloc(NAME, SIZE)
#define BROKER_GET_VALUE(NAME)\
    dheader.get_value (NAME)
#define BROKER_SET_INITVALUE(NAME, VAL)\
    dheader.set_initvalue (NAME, VAL)
#define BROKER_SET_VALUE(NAME, VAL)\
    dheader.set_value (NAME, VAL)
#define BROKER_OPEN(NAME, OFFSET, SIZE, STATEP)\
    dheader.binvar_open (NAME, OFFSET, SIZE, STATEP)
#define BROKER_CLOSE(STATE, OFFSET, SIZE)\
    dheader.binvar_close (STATE, OFFSET, SIZE)

///* printf() for iec program debugging.  plcprog.c includes iec_glue.h
//   too, and would be screwed it it tried to use printf -- but it
//   doesn't, it uses LOGI() from log.h instead. */
//#define host_printf(...)  (dheader._printf(__VA_ARGS__))

/* timer.txt and rtc.txt FBs use global __CURRENT_TIME to initialize
   their own CURRENT_TIME:DT.  Make them call a function instead.
   This macro expands both at call points and in pre-declaration
   (extern TIME __CURRENT_TIME; in iec_std_lib.h) */
#define __CURRENT_TIME  iec_glue_get_current_time_call()
IEC_TIMESPEC iec_glue_get_current_time_call (void); /* for IEC programs */

void iec_glue_apply (void *);   /* for plcprog.c */

#endif /* IEC_GLUE_H */
