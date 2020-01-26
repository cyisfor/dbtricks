cmake_minimum_required(VERSION 3.14)
project(dbtricks)

include(FindPkgConfig)
pkg_check_modules(SQLITE
  IMPORTED_TARGET
  sqlite3)

if(SQLITE_FOUND)
  if(NOT TARGET cstuff)
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/add_module")
	include(AddModule)
	add_module(cstuff
	  GIT 874ae5456e63ea82ddba9cca149600a71be06daf
	  file:///$ENV{HOME}/repo/cstuff
	  file:///$ENV{HOME}/code/cstuff
	  git@github.com:cyisfor/cstuff.git
	  https://github.com/cyisfor/cstuff.git)
  endif()
  add_cstuff(record mmapfile)

  list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

  include(all_types)
  include(restartable_transaction)

  restartable_transaction(runtime_transaction transaction transdb_handler handler void* udata)
  add_library(basedb db/base.c db/transaction.c)
  add_dependencies(basedb all_types runtime_transaction)
  target_include_directories(basedb PUBLIC
	"${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/db")
  target_link_libraries(basedb PUBLIC cstuff PkgConfig::SQLITE)

  if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
	add_executable(example example/main.c)
	target_link_libraries(example PRIVATE basedb)

	restartable_transaction(foo bar basedb_stmt insert int i int which int val char val2)
	add_executable(restartable_example
	  example/restartable.c)
	target_link_libraries(restartable_example PRIVATE basedb m)
  endif()
else(SQLITE_FOUND)
  message(WARNING "dbtricks does tricks with sqlite... which you don't seem to have.")
endif(SQLITE_FOUND)
