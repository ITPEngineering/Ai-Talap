/*
 * multivibrator at 0.5Hz
 * outputs to `out1', `out2' scalars
 * enabled by `start' scalar
 */
#include <stdint.h>
#include <stdbool.h>
#include <logicbox.h>

static int64_t next;  /* lbtime of the next action */
static bool out = false;

void run()
{
    if (lbvar ("start")) {
        if (next < lbtime()) {
            next = lbtime() + (int)0.5e6;
            lbvar_out ("out1", out);
            lbvar_out ("out2", !out);
            out = !out;
        }
    }
}

void init()
{
    next = lbtime();
}
