cmake_minimum_required(VERSION 3.14)
project(dbtricks)

function(m4 source dest implementation)
  if(implemenation)
	set(implementation "-DIMPLEMENTATION")
  endif(implementation)
  add_custom_command(
	OUTPUT "${dest}"
	COMMAND
	m4 -I"${CMAKE_CURRENT_SOURCE_DIR}" "${IMPLEMENTATION}" -P "${source}" > "${dest}"
	MAIN_DEPENDENCY
	"${source}")
endfunction(m4)

m4(db/all_types.m4.h db/all_types.c)
