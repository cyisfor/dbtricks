cmake_minimum_required(VERSION 3.14)
project(dbtricks)

function(m4 source dest implementation)
  if(implementation)
	set(derp "-DIMPLEMENTATION")
  else(implementation)
	set(derp "")
  endif(implementation)
  get_filename_component(thedir "${source}" DIRECTORY)
  add_custom_command(
	OUTPUT "${dest}"
	COMMAND
	m4 -I"${CMAKE_CURRENT_SOURCE_DIR}/m4"
	-I"${CMAKE_CURRENT_SOURCE_DIR}/${thedir}"
	"${derp}" -P "${CMAKE_CURRENT_SOURCE_DIR}/${source}" > "${dest}"
	DEPENDS
	db/all_types.m4.h
	db/types.m4.h
	MAIN_DEPENDENCY
	"${source}")
endfunction(m4)

m4(db/all_types.m4.h db/all_types.c 1)
m4(db/all_types.m4.h db/all_types.h "")

add_custom_target(all_types
  DEPENDS db/all_types.c db/all_types.h)

add_subdirectory(cstuff)
add_cstuff(itoa record mmapfile)

include(FindPkgConfig)
pkg_check_modules(SQLITE
  REQUIRED IMPORTED_TARGET
  sqlite3)

add_library(basedb db/base.c)
add_dependencies(basedb all_types)
target_include_directories(basedb PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")
target_include_directories(basedb PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/db")
target_link_libraries(basedb cstuff PkgConfig::SQLITE)

add_executable(example example/main.c)
target_link_libraries(example PRIVATE basedb)
