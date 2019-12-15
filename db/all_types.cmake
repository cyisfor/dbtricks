function (create_all_types dest generator)
  if(implementation)
	file(APPEND  \#include "all_types.h"
	  \#include <sqlite3.h>)
  else(implementation)
  file(APPEND \#include "myint.h"
	\#include "result.h")
  endif(implementation)

  set(TYPE blob)
  set(BIND_ARGS "const void* val, int len, void(*destructor)(void*)")
  set(BIND_PARAMS "val, len, destructor")
  set(COLUMN_RETURN, "const void*")
  ${generator}(${dest})
  
  set(TYPE blob64)
  set(BIND_ARGS "const void* val, u64 len, void(*destructor)(void*)")
  unset(COLUMN_RETURN)
  ${generator}(${dest})
  
  set(TYPE text)
  set(BIND_ARGS "const char* val, int len, void(*destructor)(void*)")
  set(COLUMN_RETURN "const char*")
  ${generator}(${dest})
  
  set(TYPE text64)
  set(BIND_ARGS "const char* val, u64 len, void(*destructor)(void*), unsigned char encoding")
  set(BIND_PARAMS "val, len, destructor, encoding")
  unset(COLUMN_RETURN)
  ${generator}(${dest})

  function (simple TYPE)
	if(COLUMN_RETURN)
	else()
	  set(COLUMN_RETURN "${TYPE}");
	endif()
	set(BIND_ARGS "${COLUMN_RETURN} val");
	set(BIND_PARAMS "val")
	${generator}(${dest})
	unset(COLUMN_RETURN)
  endfunction(simple)
  simple(double)
  simple(int)
  set(COLUMN_RETURN s64)
  simple(int64)

  set(TYPE zeroblob)
  set(BIND_ARGS int length)
  set(BIND_PARAMS length)
  unset(COLUMN_RETURN)
  ${generator}(${dest})

  set(TYPE zeroblob64)
  set(BIND_ARGS u64 length)
  ${generator}(${dest})
endfunction()
