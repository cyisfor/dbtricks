m4_divert(`-1');
m4_include(`c.m4')
m4_define({{N}}, {{basedb_$1}})
m4_define({{T}}, {{basedb}})
m4_divert{{}}m4_dnl ;
m4_dnl;
m4_ifdef({{IMPLEMENTATION}},{{m4_dnl;
#include "all_types.h"
}},{{m4_dnl;
#ifndef DB_ALL_TYPES_H
#define DB_ALL_TYPES_H
#include "myint.h"
#include "result.h"			
}})m4_dnl;

m4_divert({{-1}});
m4_define({{TYPE}}, blob);
m4_define({{BIND_ARGS}}, {{const void* val, int len, void(*destructor)(void*)}});
m4_define({{BIND_PARAMS}}, {{val, len, destructor}});
m4_define({{COLUMN_RETURN}}, {{const void*}})
m4_divert{{}}m4_dnl ;
m4_include({{types.m4.h}})m4_dnl;
m4_dnl;
m4_divert({{-1}});
m4_define({{TYPE}}, blob64)
m4_define({{BIND_ARGS}}, {{const void* val, u64 len, void(*destructor)(void*)}})
m4_divert{{}}m4_dnl ;
m4_include({{types.m4.h}})m4_dnl;
m4_dnl;
m4_divert({{-1}});
m4_define({{TYPE}}, text)
m4_define({{BIND_ARGS}}, {{const char* val, int len, void(*destructor)(void*)}})
m4_define({{COLUMN_RETURN}}, {{const char*}})
m4_divert{{}}m4_dnl ;
m4_include({{types.m4.h}})m4_dnl;
m4_dnl;
m4_divert({{-1}});
m4_define({{TYPE}}, text64);
m4_define({{BIND_ARGS}}, {{const char* val, u64 len, void(*destructor)(void*), unsigned char encoding}});
m4_define({{BIND_PARAMS}}, {{val, len, destructor, encoding}});
m4_divert{{}}m4_dnl ;
m4_include({{types.m4.h}})m4_dnl;
m4_dnl;
m4_divert({{-1}});
m4_define({{TYPE}}, double);
m4_define({{COLUMN_RETURN}}, {{TYPE}});
m4_define({{BIND_ARGS}}, {{COLUMN_RETURN val}});
m4_define({{BIND_PARAMS}}, {{val}});
m4_divert{{}}m4_dnl ;
m4_include({{types.m4.h}})m4_dnl;
m4_dnl;
m4_divert({{-1}});
m4_define({{TYPE}}, int);
m4_divert{{}}m4_dnl ;
m4_include({{types.m4.h}})m4_dnl;
m4_dnl;
m4_divert({{-1}});
m4_define({{TYPE}}, int64);
m4_define({{COLUMN_RETURN}}, {{s64}});
m4_divert{{}}m4_dnl ;
m4_include({{types.m4.h}})m4_dnl;
m4_dnl;
m4_divert({{-1}});
m4_define({{TYPE}}, zeroblob);
m4_undefine({{COLUMN_RETURN}});
m4_define({{BIND_ARGS}}, {{int val}});
m4_divert{{}}m4_dnl ;
m4_include({{types.m4.h}})m4_dnl;
m4_dnl;
m4_divert({{-1}});
m4_define({{TYPE}}, zeroblob64);
m4_define({{BIND_ARGS}}, {{u64 val}});
m4_divert{{}}m4_dnl ;
m4_include({{types.m4.h}})m4_dnl;

m4_ifdef({{IMPLEMENTATION}},{{}},{{m4_dnl;
#endif /* DB_ALL_TYPES_SNIPPET_H */
}})m4_dnl;

