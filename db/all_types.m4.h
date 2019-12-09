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
#define BIND_ARGS const void* val, u64 len, void(*destructor)(void*) 
#define BIND_PARAMS val, len, destructor
#include "types.snippet.h"
#define TYPE text
#define BIND_ARGS const char* val, int len, void(*destructor)(void*) 
#define BIND_PARAMS val, len, destructor
#define COLUMN_RETURN const void*
#include "types.snippet.h"
#define TYPE text64
#define BIND_ARGS const char* val, u64 len, void(*destructor)(void*), unsigned char encoding
#define BIND_PARAMS val, len, destructor, encoding
#include "types.snippet.h"

#define TYPE double
#define BIND_ARGS double val
#define BIND_PARAMS val
#define COLUMN_RETURN double
#include "types.snippet.h"
#define TYPE int
#define BIND_ARGS int val
#define BIND_PARAMS val
#define COLUMN_RETURN int
#include "types.snippet.h"
#define TYPE int64
#define BIND_ARGS s64 val
#define BIND_PARAMS val
#define COLUMN_RETURN s64
#include "types.snippet.h"

#define TYPE zeroblob
#define BIND_ARGS int n
#define BIND_PARAMS n
#include "types.snippet.h"
#define TYPE zeroblob64
#define BIND_ARGS u64 n
#define BIND_PARAMS n
#include "types.snippet.h"
#ifdef IMPLEMENTATION
#undef IMPLEMENTATION
#endif
#endif /* DB_ALL_TYPES_SNIPPET_H */
