#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <plcprog_c_glue.h>

#define lbabort()\
  lbabort__ (__FILE__, __LINE__)
#define lblog(FMT, ...)\
  lblog__ (__FILE__, __LINE__, FMT __VA_OPT__(,) __VA_ARGS__)

#include <atyield.h>            /* uses lbabort() */

#ifndef htonl
#  include <machine/endian.h>
#  define htonl(X)  __htonl(X)
#  define htons(X)  __htons(X)
#  define ntohl(X)  __ntohl(X)
#  define ntohs(X)  __ntohs(X)
#endif /* htonl */

#ifndef snprintf
#define snprintf(...) (lbsnprintf (__VA_ARGS__))
#endif /* snprintf */

/* error codes returned by lbmodbus() and lb485(), directly or in resvar */
enum logicbox_ret {
    LOGICBOX_OK = 0,

    /* Modbus exception codes (negated).  Cf. enum bbm_exception_code */
    LOGICBOX_ILLEGAL_FUNCTION = -1,
    LOGICBOX_ILLEGAL_DATA_ADDRESS = -2,
    LOGICBOX_ILLEGAL_DATA_VALUE = -3,
    LOGICBOX_SLAVE_DEVICE_FAILURE = -4,
    LOGICBOX_ACKNOWLEDGE = -5,
    LOGICBOX_SLAVE_DEVICE_BUSY = -6,
    LOGICBOX_MEMORY_PARITY_ERROR = -8,
    LOGICBOX_GATEWAY_PATH_UNAVAILABLE = -0xa,
    LOGICBOX_GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND = -0xb,

    /* internal errors.  Cf. enum modbus_client_ret */
    LOGICBOX_ERR_LOGGED = -1000,      /* none of the below, see lblog */
    LOGICBOX_ERR_INVAL = -999,        /* invalid op args */
    LOGICBOX_ERR_VAR = -998,          /* !var_valid in write ops */
    LOGICBOX_ERR_VARSHORT = -997,     /* insufficient var data length */
    LOGICBOX_ERR_CONNECT = -996,      /* cannot connect to the server */
    LOGICBOX_ERR_EXCEPTION0 = -995,   /* exception with code=0 received */
    LOGICBOX_ERR_FN_MISMATCH = -994,  /* wrong fn code in the response */
    LOGICBOX_ERR_RESPADDR = -993,     /* wrong regaddr in the response */
    LOGICBOX_ERR_RESPLEN = -992,      /* improper response length */
    LOGICBOX_ERR_RESPDATA = -991,     /* improper data in the response */
    LOGICBOX_ERR_SHORTWRITE =  -990,  /* "cannot happen" */
    LOGICBOX_ERR_TIMEOUT = -989,      /* response_timeout reached */
    LOGICBOX_ERR_REQLEN = -988,       /* request data too long */
    LOGICBOX_ERR_NOTSUPP = -987,      /* not supported in this devtype */
    LOGICBOX_ERR_OOM = -986,          /* OOM while handling the request */
    LOGICBOX_ERR_QUEUE = -985,        /* modbus_client queue overflow */
    LOGICBOX_ERR_TOOLONG = -984,      /* data length exceeds MAXPDU */
    LOGICBOX_ERR_RESVAR = -983,       /* lbmodbus resvar not present in conf */
    LOGICBOX_ERR_NXPORT = -982,       /* invalid PORT arg */
};


extern struct plcprog_c_dheader dheader; /* from headers.c */


static inline int
lb485(int port, const void *data, int wlen, int rlen, const char *resvar, int timeout)
{
    int (*f)(int, const void *, int, int, const char *, int) = dheader.dispatch ("lb485");
    return f (port, data, wlen, rlen, resvar, timeout);
}


static inline void
lbabort__(const char *file, int line)
{
    void (*f)(const char *file, int line) = dheader.dispatch ("lbabort");
    f (file, line);
}


static inline int
lbbin (const char *name, void *buf, size_t bufsz)
{
    int (*f)(const char *, void *, size_t) = dheader.dispatch ("lbbin");
    return f (name, buf, bufsz);
}

static inline void
lbbin_out (const char *name, const void *buf, size_t len)
{
    void (*f)(const char *, const void *, size_t) = dheader.dispatch ("lbbin_out");
    f (name, buf, len);
}

static inline void
lbbin_init (const char *name, const void *buf, size_t len)
{
    void (*f)(const char *, const void *, size_t) = dheader.dispatch ("lbbin_init");
    f (name, buf, len);
}

/* lbint is lbint64 in disguise.  Convenient like "lbvar" but int, not double */
static inline int
lbint (const char *name)
{
    int64_t (*f)(const char *name) = dheader.dispatch ("lbint64");
    return (int)f (name);
}

static inline void
lbint_init (const char *name, int val)
{
    void (*f)(const char *, int64_t) = dheader.dispatch ("lbint64_init");
    f (name, val);
}

static inline void
lbint_out (const char *name, int val)
{
    void (*f)(const char *, int64_t) = dheader.dispatch ("lbint64_out");
    f (name, val);
}


static inline int64_t
lbint64 (const char *name)
{
    int64_t (*f)(const char *name) = dheader.dispatch ("lbint64");
    return f (name);
}

static inline void
lbint64_init (const char *name, int64_t val)
{
    void (*f)(const char *, int64_t) = dheader.dispatch ("lbint64_init");
    f (name, val);
}

static inline void
lbint64_out (const char *name, int64_t val)
{
    void (*f)(const char *, int64_t) = dheader.dispatch ("lbint64_out");
    f (name, val);
}


static inline void
lblog__(const char *file, int line, const char *fmt, ...)
{
    void (*f)(const char *, int, const char *, va_list ap) = dheader.dispatch ("lblog");
    va_list ap;
    va_start (ap, fmt);
    f (file, line, fmt, ap);
    va_end (ap);
}


static inline int
lbmodbus(int portindex, int addr, int fn, int regaddr, int quantity,
         const void *data, const char *resvar, int timeout)
{
    int (*f)(int, int, int, int, int, const void *, const char *, int) =
        dheader.dispatch ("lbmodbus");
    return f (portindex, addr, fn, regaddr, quantity, data, resvar, timeout);
}


static inline int
lbmodbus_crc (const void *buf, size_t bufsz)
{
    int (*f)(const void *, size_t) = dheader.dispatch ("lbmodbus_crc");
    return f (buf, bufsz);
}


static inline int
lbsnprintf(char *buf, int sz, const char *fmt, ...)
{
    int (*f)(char *, int, const char *, va_list ap) =
        dheader.dispatch ("lbsnprintf");
    va_list ap;
    va_start (ap, fmt);
    int ret = f (buf, sz, fmt, ap);
    va_end (ap);
    return ret;
}


static inline int64_t
lbtime ()
{
    int64_t (*f)(void) = dheader.dispatch ("lbtime");
    return f ();
}


static inline uint64_t
lbuint64 (const char *name)
{
    uint64_t (*f)(const char *name) = dheader.dispatch ("lbuint64");
    return f (name);
}

static inline void
lbuint64_init (const char *name, uint64_t val)
{
    uint64_t (*f)(const char *, uint64_t) = dheader.dispatch ("lbuint64_init");
    f (name, val);
}

static inline void
lbuint64_out (const char *name, uint64_t val)
{
    uint64_t (*f)(const char *, uint64_t) = dheader.dispatch ("lbuint64_out");
    f (name, val);
}


static inline double
lbvar (const char *name)
{
    double (*f)(const char *name) = dheader.dispatch ("lbvar");
    return f (name);
}

static inline void
lbvar_init (const char *name, double val)
{
    void (*f)(const char *name, double val) = dheader.dispatch ("lbvar_init");
    f (name, val);
}

static inline void
lbvar_out (const char *name, double val)
{
    void (*f)(const char *name, double val) = dheader.dispatch ("lbvar_out");
    f (name, val);
}
