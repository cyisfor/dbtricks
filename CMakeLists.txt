cmake_minimum_required(VERSION 3.14)
project(dbtricks)

configure_file(db/all_types.cmake.h db/all_types.h)
configure_file(db/all_types.cmake.c db/all_types.c)

add_custom_target(all_types
  DEPENDS db/all_types.c db/all_types.h)

add_subdirectory(cstuff)
add_cstuff(itoa record mmapfile)

include(FindPkgConfig)
pkg_check_modules(SQLITE
  REQUIRED IMPORTED_TARGET
  sqlite3)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(all_types)
include(restartable_transaction)

restartable_transaction(runtime_transaction transaction transdb_handler handler void* udata)
add_custom_target(inner_include_crap
  DEPENDS runtime_transaction.c)
add_library(basedb db/base.c db/transaction.c)
add_dependencies(basedb all_types inner_include_crap)
target_include_directories(basedb PUBLIC
  "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/db")
target_link_libraries(basedb cstuff PkgConfig::SQLITE)

add_executable(example example/main.c)
target_link_libraries(example PRIVATE basedb)

restartable_transaction(foo bar basedb_stmt insert int i int which int val char val2)
add_custom_target(inner_include_crap2
  DEPENDS foo.c)
add_executable(restartable_example
  example/restartable.c)
add_dependencies(restartable_example inner_include_crap2)
target_link_libraries(restartable_example PRIVATE basedb m)
