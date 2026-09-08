#include <stdint.h>
#include <ai_talap.h>

int res(int in) {
  int out = 0;
  if (in == 1) out = 1;
  else out = 0;
  return out;
}

void run() {
  //if (lbint("di2") == 1) lbvar_out("do0", 1);
  //else lbvar_out("do0", 0);
  //int i = lbint("di2");
  //lbvar_out("do0", 1);
  lbvar_out("do0", res(lbint("di2")));
}

void init() {
  //lbint_init("di2", 0); 
  //lbint_init("do0", 0);

}
