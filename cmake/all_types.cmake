if(DEST)
  message("Writing to ${DEST}")
  file(WRITE "${DEST}")
  function (create_one_type)
	list(JOIN BIND_ARGS ", " BIND_ARGS)
	list(JOIN BIND_PARAMS ", " BIND_PARAMS)
	if(IMPLEMENTATION)	
	  file(APPEND "${DEST}" "EXPORT
result N(bind_${TYPE})(N(stmt) stmt, int col, ${BIND_ARGS}) {
	return check(stmt->db,
		sqlite3_bind_${TYPE}(stmt->sqlite, col, ${BIND_PARAMS}));
}
")
	  if(COLUMN_RETURN)
		file(APPEND "${DEST}" "EXPORT
${COLUMN_RETURN} N(column_${TYPE})(N(stmt) stmt, int col) {
		return sqlite3_column_${TYPE}(stmt->sqlite, col);
}
")
	  endif(COLUMN_RETURN)
	else(IMPLEMENTATION)
	  file(APPEND "${DEST}" "result N(bind_${TYPE})(N(stmt) stmt, int col, ${BIND_ARGS});
")
	  if(COLUMN_RETURN)
		file(APPEND "${DEST}" "${COLUMN_RETURN} N(column_${TYPE})(N(stmt), int col);

")
	  endif(COLUMN_RETURN)
	endif(IMPLEMENTATION)
  endfunction(create_one_type)

  if(implementation)
	file(APPEND "${DEST}"  "#include \"all_types.h\"
#include <sqlite3.h>

")
  else(implementation)
	file(APPEND "${DEST}" "#include \"myint.h\"
#include \"result.h\"

")
  endif(implementation)

  set(TYPE blob)
  set(BIND_ARGS "const void* val" "int len" "void(*destructor)(void*)")
  set(BIND_PARAMS val len destructor)
  set(COLUMN_RETURN, "const void*")
  create_one_type()
  
  set(TYPE blob64)
  set(BIND_ARGS "const void* val" "u64 len" "void(*destructor)(void*)")
  unset(COLUMN_RETURN)
  create_one_type()
  
  set(TYPE text)
  set(BIND_ARGS "const char* val" "int len" "void(*destructor)(void*)")
  set(COLUMN_RETURN "const char*")
  create_one_type()
  
  set(TYPE text64)
  set(BIND_ARGS "const char* val" "u64 len" "void(*destructor)(void*)" "unsigned char encoding")
  set(BIND_PARAMS val len destructor encoding)
  unset(COLUMN_RETURN)
  create_one_type()

  function (simple TYPE)
	if(COLUMN_RETURN)
	else(COLUMN_RETURN)
	  set(COLUMN_RETURN "${TYPE}")
	endif()
	set(BIND_ARGS "${COLUMN_RETURN} val")
	set(BIND_PARAMS "val")
	create_one_type()
	unset(COLUMN_RETURN)
  endfunction(simple)
  simple(double)
  simple(int)
  set(COLUMN_RETURN s64)
  simple(int64)

  set(TYPE zeroblob)
  set(BIND_ARGS "int length")
  set(BIND_PARAMS length)
  unset(COLUMN_RETURN)
  create_one_type()

  set(TYPE zeroblob64)
  set(BIND_ARGS "u64 length")
  create_one_type()
else(DEST)
  add_custom_command(
	OUTPUT "db/all_types.c"
	COMMAND
	cmake -DDEST=db/all_types.c -DIMPLEMENTATION=TRUE -P "${CMAKE_CURRENT_LIST_FILE}"
	DEPENDS "${CMAKE_CURRENT_LIST_FILE}")
  add_custom_command(
	OUTPUT "db/all_types.h"
	COMMAND
	cmake -DDEST=db/all_types.h -P "${CMAKE_CURRENT_LIST_FILE}"
	DEPENDS "${CMAKE_CURRENT_LIST_FILE}")
  add_custom_target(
	all_types
	DEPENDS "db/all_types.c" "db/all_types.h")
endif(DEST)
