#!/usr/bin/perl

my @fnnames;
while (<>) {
    # void TEST1_init__(TEST1 *data__, BOOL retain) {
    # void TEST1_body__(TEST1 *data__) {
    if (/^void (\w+)/) {
        push @fnnames, $1;
    }
}

@fnnames == 2 or die "hmmm... none or more than one PROGRAM in the source?";

my $objtype = ($fnnames[0] =~ s/_init__//r);

while (<DATA>) {
    s/%INIT%/$fnnames[0]/;
    s/%RUN%/$fnnames[1]/;
    s/%OBJTYPE%/$objtype/;
    print;
}

exit 0;

__DATA__
#include "iec_glue.h"           /* for struct dheader */
#include "POUS.h"               /* for `objtype' (e.g. TEST1) */

struct theader __attribute__((section(".theader"))) header = {
    .magic = IEC_GLUE_MAGIC,
    .objsz = sizeof(%OBJTYPE%),
    .init = (void*)%INIT%,      /* obj types not known to plcprog.c ... */
    .run = (void*)%RUN%,        /* ... have to cast to void* */
};

/* .data segment header with pointers to firmware functions callable
   from IEC program.  No init here.  The header is filled with actual
   pointers to functions in plcprog.c when loading .data of an IEC
   program in RAM */
struct dheader __attribute__((section(".dheader"))) dheader;

/* iec_glue.h arranged for this fn to be called by users
   of __CURRENT_TIME (timer.txt and rtc.txt FBs) */
IEC_TIMESPEC
iec_glue_get_current_time_call ()
{
    return dheader.get_current_time();
}
