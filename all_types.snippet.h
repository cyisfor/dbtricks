#ifndef DB_ALL_TYPES_SNIPPET_H
#define DB_ALL_TYPES_SNIPPET_H
#include "concatsym.h"
#include "myint.h"

#define TYPE blob
#define BIND_ARGS const void* val, int len, void(*destructor)(void*) 
#define BIND_PARAMS val, len, destructor
#define COLUMN_RETURN const void*
#include "types.snippet.h"
#define TYPE blob64
#undef BIND_ARGS
#define BIND_ARGS const void* blob, u64 len, void(*destructor)(void*)
#include "types.snippet.h"
#define TYPE text
#undef BIND_ARGS
#define BIND_ARGS const char* val, int len, void(*destructor)(void*)
#undef COLUMN_RETURN
#define COLUMN_RETURN const char*
#include "types.snippet.h"
#define TYPE text64
#undef BIND_ARGS
#define BIND_ARGS const char* val, u64 len, void(*destructor)(void*)
#include "types.snippet.h"
#undef BIND_ARGS
#define BIND_ARGS TYPE val
#undef BIND_PARAMS
#define BIND_PARAMS val
#undef COLUMN_RETURN
#define COLUMN_RETURN TYPE
#define TYPE double
#include "types.snippet.h"
#define TYPE int
#include "types.snippet.h"
#define TYPE int64
#undef COLUMN_RETURN
#define COLUMN_RETURN s64
#undef BIND_ARGS
#define BIND_ARGS s64 val
#include "types.snippet.h"
#define TYPE zeroblob
#undef COLUMN_RETURN
#define COLUMN_RETURN void*
#undef BIND_ARGS
#define BIND_ARGS int val
#include "types.snippet.h"
#define TYPE zeroblob64
#undef BIND_ARGS
#define BIND_ARGS s64 val
#include "types.snippet.h"

#undef BIND_ARGS
#undef BIND_PARAMS
#undef COLUMN_RETURN
#ifdef IMPLEMENTATION
#undef IMPLEMENTATION
#endif
#endif /* DB_ALL_TYPES_SNIPPET_H */
