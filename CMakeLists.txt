cmake_minimum_required(VERSION 3.14)
project(dbtricks)

function(m4 source dest implementation)
  if(implementation)
	set(implementation "-DIMPLEMENTATION")
  endif(implementation)
  get_filename_component(thedir "${source}" DIRECTORY)
  message(INFO foo (${thedir})(${source}))
  add_custom_command(
	OUTPUT "${dest}"
	COMMAND
	m4 -I"${CMAKE_CURRENT_SOURCE_DIR}/m4"
	-I"${CMAKE_CURRENT_SOURCE_DIR}/${thedir}"
	"${IMPLEMENTATION}" -P "${CMAKE_CURRENT_SOURCE_DIR}/${source}" > "${dest}"
	MAIN_DEPENDENCY
	"${source}")
endfunction(m4)

m4(db/all_types.m4.h db/all_types.c 1)
m4(db/all_types.m4.h db/all_types.h "")

add_executable(derp db/all_types.c)
