cmake_minimum_required(VERSION 3.14)
project(dbtricks)

function(m4 source)
  set(options IMPLEMENTATION)
  set(single DEST EXT)
  set(multiple OPTIONS DEPENDS)
  cmake_parse_arguments(PARSE_ARGV 1 A "${options}" "${single}" "${multiple}")

  if(A_IMPLEMENTATION)
	list(APPEND A_OPTIONS "-DIMPLEMENTATION")
	if(!A_EXT AND !A_DEST)
	  set(A_EXT "c")
	endif()
  else()
	if(!A_EXT AND !A_DEST)
	  set(A_EXT "h")
	endif()
  endif(A_IMPLEMENTATION)
  get_filename_component(thedir "${source}" DIRECTORY)
  if(!A_DEST)
	get_filename_component(A_DEST "${source}" NAME_WE)
	set(A_DEST "${thedir}/${A_DEST}.${A_EXT}")
  endif()
  add_custom_command(
	OUTPUT "${dest}"
	COMMAND
	m4 -I"${CMAKE_CURRENT_SOURCE_DIR}/m4"
	-I"${CMAKE_CURRENT_SOURCE_DIR}/${thedir}"
	${A_OPTIONS}
	-P "${CMAKE_CURRENT_SOURCE_DIR}/${source}" > "${A_DEST}"
	DEPENDS
	${A_DEPENDS}
	MAIN_DEPENDENCY
	"${source}")
endfunction(m4)

m4(db/all_types.m4.h IMPLEMENTATION)
m4(db/all_types.m4.h db/all_types.h "")

function(restartable_module target function argument)
  

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
