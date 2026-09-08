#pragma once

/*
 * our little yield/resume implementation
 * 
 * Inspired by protothreads:
 *   http://dunkels.com/adam/pt/
 *   https://github.com/LarryRuane/protothread
 */

enum {LBCALL_MAXNEST = 5};
extern void *lbresume__[LBCALL_MAXNEST];
extern int lblevel__;

#define lbyield(...)\
  do {\
      __label__ label;\
      lbresume__[lblevel__] = &&label;\
      return __VA_ARGS__;\
    label:\
  } while(0)

#define lbresume()\
  do {\
      if (lbresume__[lblevel__]) {\
          void *label = lbresume__[lblevel__];\
          lbresume__[lblevel__] = NULL;\
          goto *label;\
      }\
  } while(0)

#define lbcall(CALLEXPR, ...)\
  ({\
      __label__ label;\
    label:\
      if (lblevel__ >= LBCALL_MAXNEST-1) {\
          lbabort();\
      }\
      lblevel__++;\
      __auto_type ret = (CALLEXPR);\
      if (lbresume__[lblevel__]) {/*callee yielded*/\
          lbresume__[--lblevel__] = &&label;\
          return __VA_ARGS__;\
      }\
      else {\
          lblevel__--;\
      }\
      ret;\
  })

/* this has to be used if CALLEXPR is a void expression */
#define lbcall_void(CALLEXPR, ...)\
  ({\
      __label__ label;\
    label:\
      if (lblevel__ >= LBCALL_MAXNEST-1) {\
          lbabort();\
      }\
      lblevel__++;\
      (CALLEXPR);\
      if (lbresume__[lblevel__]) {/*callee yielded*/\
          lbresume__[--lblevel__] = &&label;\
          return __VA_ARGS__;\
      }\
      else {\
          lblevel__--;\
      }\
  })
