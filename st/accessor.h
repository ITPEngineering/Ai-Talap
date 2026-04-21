#ifndef __ACCESSOR_H
#define __ACCESSOR_H

#include <inttypes.h>
#include "iec_glue.h"

#define __INITIAL_VALUE(...) __VA_ARGS__

// variable declaration macros
#define __DECLARE_VAR(type, name)\
	__IEC_##type##_t name;
#define __DECLARE_GLOBAL(type, domain, name)\
	__IEC_##type##_t domain##__##name;\
	static __IEC_##type##_t *GLOBAL__##name = &(domain##__##name);\
	void __INIT_GLOBAL_##name(type value) {\
		(*GLOBAL__##name).value = value;\
	}\
	IEC_BYTE __IS_GLOBAL_##name##_FORCED(void) {\
		return (*GLOBAL__##name).flags & __IEC_FORCE_FLAG;\
	}\
	type* __GET_GLOBAL_##name(void) {\
		return &((*GLOBAL__##name).value);\
	}  error Not implemented
#define __DECLARE_GLOBAL_FB(type, domain, name)\
	type domain##__##name;\
	static type *GLOBAL__##name = &(domain##__##name);\
	type* __GET_GLOBAL_##name(void) {\
		return &(*GLOBAL__##name);\
	}\
	extern void type##_init__(type* data__, BOOL retain);  error Not implemented
#define __DECLARE_GLOBAL_LOCATION(type, location)\
	extern type *location;  error Not implemented
#define __DECLARE_GLOBAL_LOCATED(type, resource, name)\
	__IEC_##type##_p resource##__##name;\
	static __IEC_##type##_p *GLOBAL__##name = &(resource##__##name);\
	void __INIT_GLOBAL_##name(type value) {\
		*((*GLOBAL__##name).value) = value;\
	}\
	IEC_BYTE __IS_GLOBAL_##name##_FORCED(void) {\
		return (*GLOBAL__##name).flags & __IEC_FORCE_FLAG;\
	}\
	type* __GET_GLOBAL_##name(void) {\
		return (*GLOBAL__##name).value;\
	}  error Not implemented
#define __DECLARE_GLOBAL_PROTOTYPE(type, name)\
    extern type* __GET_GLOBAL_##name(void);  error Not implemented

/* External var live in broker's vtab.  The empty name[0] array is
   used in __GET_EXTERNAL to cast the value retrieved from vtab to the
   type expected by the caller.  TODO: might cache the blob pointer in
   INIT to improve GET/SET efficiency */
#define __DECLARE_EXTERNAL(type, name)\
        type name[0]; /* type only, the value in vtab */

#define __DECLARE_EXTERNAL_FB(type, name)\
	type* name;  error Not implemented

/* tentative... unlike external ones, located vars can have
   initializers which are useful.  We don't have real locations, all
   interaction goes via broker.  Use with dummy ones? */
#define __DECLARE_LOCATED(type, name)\
    __DECLARE_EXTERNAL(type, name)\

/* Re-define __DECLARE_STRUCT_TYPE (originally in iec_types_all.h) to
   make packed struct which is less efficient but more predictable for
   alising which PLC programmers frequently employ */
#undef __DECLARE_STRUCT_TYPE
#define __DECLARE_STRUCT_TYPE(type, elements)\
typedef struct __attribute__((packed)) {\
  elements\
} type;\
__DECLARE_COMPLEX_STRUCT(type)


// variable initialization macros
// retained=TRUE means a warm restart is underway, use retained values.
// But original macros only use it to set __IEC_RETAIN_FLAG in the variable
// flags and nothing more
#define __INIT_RETAIN(name, retained)\
    name.flags |= retained?__IEC_RETAIN_FLAG:0; /* TODO retain */
#define __INIT_VAR(name, initial, retained)\
	name.value = initial;\
	__INIT_RETAIN(name, retained)  /* TDOO retain */
#define __INIT_GLOBAL(type, name, initial, retained)\
    {\
	    type temp = initial;\
	    __INIT_GLOBAL_##name(temp);\
	    __INIT_RETAIN((*GLOBAL__##name), retained)\
    }  error Not implemented
#define __INIT_GLOBAL_FB(type, name, retained)\
	type##_init__(&(*GLOBAL__##name), retained);  error Not implemented
#define __INIT_GLOBAL_LOCATED(domain, name, location, retained)\
	domain##__##name.value = location;\
	__INIT_RETAIN(domain##__##name, retained)  error Not implemented

/* Note: `name' is actually `prefix_name' here, e.g. data__->MYREC.
   Unlike __SET_EXTERNAL, telling aggregate types from scalars is not
   easy here.  Q&D for now, TODO: recode to be optimizable. TODO:
   LWORD & friends won't fit double, differentiate */
#define __INIT_EXTERNAL(type, global, name, retained)\
    if (strcmp (#type, "BYTE") == 0 ||\
        strcmp (#type, "WORD") == 0 ||\
        strcmp (#type, "DWORD") == 0 ||\
        strcmp (#type, "LWORD") == 0 ||\
        strcmp (#type, "LREAL") == 0 ||\
        strcmp (#type, "REAL") == 0 ||\
        strcmp (#type, "SINT") == 0 ||\
        strcmp (#type, "INT") == 0 ||\
        strcmp (#type, "DINT") == 0 ||\
        strcmp (#type, "LINT") == 0 ||\
        strcmp (#type, "USINT") == 0 ||\
        strcmp (#type, "UINT") == 0 ||\
        strcmp (#type, "UDINT") == 0 ||\
        strcmp (#type, "ULINT") == 0 ||\
        strcmp (#type, "BOOL") == 0) {\
        /* scalar type */\
        BROKER_SCALAR_CHECK(#global);\
    }\
    else {\
        BROKER_BINVAR_ALLOC(#global, sizeof (type));\
    }

#define __INIT_EXTERNAL_FB(type, global, name, retained)\
	name = __GET_GLOBAL_##global();  error Not implemented

/* Note: `name' is actually `prefix_name' here, e.g. data__->MYREC.
   `location' is like __IX0_0, useless, ignore.  */
#define __INIT_LOCATED(type, location, name, retained)\
        __INIT_EXTERNAL(type, #name+8, name, retained)

#define __INIT_LOCATED_VALUE(name, initial)\
        BROKER_SET_INITVALUE (#name+8, initial);


// variable getting macros
#define __GET_VAR(name, ...)\
	name.value __VA_ARGS__

#define __GET_EXTERNAL(name, suffix)\
    ({\
        typedef __typeof__ (name[0] suffix) field_t;\
        field_t res;\
        /* #name includes the prefix: "__data->name" */\
        char *name_sans_prefix = #name + 8;\
        if (#suffix[0] == '\0') {/* empty suffix => scalar type */\
            res = (field_t)BROKER_GET_VALUE(name_sans_prefix);\
        }\
        else {\
            typedef __typeof__ (name[0]) var_t;\
            var_t *zerop = (var_t*)0;\
            size_t offset = (size_t) &((*zerop)suffix);\
            size_t size = sizeof ((*zerop)suffix);\
            void *state;\
            var_t *p = BROKER_OPEN (name_sans_prefix, offset, size, &state);\
            res = (*p)suffix;\
            BROKER_CLOSE (state, offset, size);\
        }\
        res;\
    })

#define __GET_EXTERNAL_FB(name, ...)\
	__GET_VAR(((*name) __VA_ARGS__))  error Not implemented
#  define __GET_LOCATED(name, ...)\
	(__typeof__ (name##[0])) BROKER_GET_VALUE(#name, show_me_error##__VA_ARGS__)
#define __GET_VAR_BY_REF(name, ...)\
	(&(name.value __VA_ARGS__))  error Not implemented
#define __GET_EXTERNAL_BY_REF(name, ...)\
	(&((*(name.value)) __VA_ARGS__))  error Not implemented
#define __GET_EXTERNAL_FB_BY_REF(name, ...)\
	__GET_EXTERNAL_BY_REF(((*name) __VA_ARGS__))  error Not implemented
#define __GET_LOCATED_BY_REF(name, ...)\
	(&((*(name.value)) __VA_ARGS__))  error Not implemented

#define __GET_VAR_REF(name, ...)\
	(&(name.value __VA_ARGS__))  error Not implemented
#define __GET_EXTERNAL_REF(name, ...)\
	(&((*(name.value)) __VA_ARGS__))  error Not implemented
#define __GET_EXTERNAL_FB_REF(name, ...)\
	(&(__GET_VAR(((*name) __VA_ARGS__))))  error Not implemented
#define __GET_LOCATED_REF(name, ...)\
	(&((*(name.value)) __VA_ARGS__))  error Not implemented

#define __GET_VAR_DREF(name, ...)\
	(*(name.value __VA_ARGS__))  error Not implemented
#define __GET_EXTERNAL_DREF(name, ...)\
	(*((*(name.value)) __VA_ARGS__))  error Not implemented
#define __GET_EXTERNAL_FB_DREF(name, ...)\
	(*(__GET_VAR(((*name) __VA_ARGS__)))) error Not implemented
#define __GET_LOCATED_DREF(name, ...)\
	(*((*(name.value)) __VA_ARGS__)) error Not implemented


// variable setting macros
#define __SET_VAR(prefix, name, suffix, new_value)\
	if (!(prefix name.flags & __IEC_FORCE_FLAG)) prefix name.value suffix = new_value

/* scalar values are simply assigned to vtab elements.  Aggregate
   field assignments are patched into the binary blob associated with
   the aggregate variable in vtab.  Attention -- vtab_blob_lock is
   taken by BROKER_OPEN and held till BROKER_CLOSE */
#define __SET_EXTERNAL(prefix, name, suffix, new_value)\
        if (#suffix[0] == '\0') {/* empty suffix => scalar type */\
            BROKER_SET_VALUE (#name, (double)(new_value));\
        }\
        else {\
            typedef __typeof__ (prefix name[0]) var_t;\
            var_t *zerop = (var_t*)0;\
            size_t offset = (size_t) &((*zerop)suffix);\
            size_t size = sizeof ((*zerop)suffix);\
            void *state;\
            var_t *p = BROKER_OPEN (#name, offset, size, &state);\
            (*p)suffix = new_value;\
            BROKER_CLOSE (state, offset, size);\
        }

#define __SET_EXTERNAL_FB(prefix, name, suffix, new_value)\
	__SET_VAR(prefix, name, suffix, new_value)  error Not implemented

#define __SET_LOCATED(prefix, name, suffix, new_value)\
        BROKER_SET(prefix name.symref, (double)(new_value))

#endif //__ACCESSOR_H
