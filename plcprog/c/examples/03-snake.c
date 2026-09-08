/*
 * snake example:
 * outputs running "1" to `mw1' scalar
 * enabled by `exec' scalar
 * increments `inc1' scalar on every cycle
 * cycles to test maxrun behaviour if `stop1' is true
 */
#include <stdint.h>
#include <logicbox.h>

void run()
{
    if (lbvar ("exec")) {
        uint16_t mw1 = lbvar ("mw1");
        mw1 <<= 1;
        lbvar_out ("mw1", mw1? mw1: 1);

        do {
            lbvar_out ("inc1", lbvar ("inc1") + 1);
        } while (lbvar ("stop1"));

	if ((int)lbvar ("inc1") % 20 == 0) {
            lblog ("checkpoint %g", lbvar ("inc1"));
        }
//	if ((int)lbvar ("inc1") % 80 == 0) {
//            lbabort();
//        }
    }
}

void init()
{
    lbvar_init ("inc1", 0);
    lbvar_init ("mw1", 0);
}
