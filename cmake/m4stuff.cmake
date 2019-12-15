function(m4 source)
  set(options IMPLEMENTATION)
  set(single DEST EXT)
  set(multiple OPTIONS DEPENDS)
  cmake_parse_arguments(PARSE_ARGV 1 A "${options}" "${single}" "${multiple}")

  if(A_IMPLEMENTATION)
	list(APPEND A_OPTIONS "-DIMPLEMENTATION")
  endif(A_IMPLEMENTATION)
  get_filename_component(thedir "${source}" DIRECTORY)
  if(A_DEST)
  else()
	if(A_EXT)
	else()
	  if(A_IMPLEMENTATION)
		set(A_EXT "c")
	  else(A_IMPLEMENTATION)
		set(A_EXT "h")
	  endif(A_IMPLEMENTATION)
	endif()
	
	get_filename_component(A_DEST "${source}" NAME_WE)
	set(A_DEST "${thedir}/${A_DEST}.${A_EXT}")
  endif()
  add_custom_command(
	OUTPUT "${A_DEST}"
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
m4(db/all_types.m4.h)

function(restartable_transaction target function)
  foreach(thing ${ARGN})
	if(type)
	  list(APPEND arguments "${type} ${thing}")
	  list(APPEND values "${thing}")
	  unset(type)
	else(type)
	  set(type "${thing}")
	endif(type)
  endforeach()
  list(JOIN arguments ", " arguments)
  list(JOIN values ", " values)
  if(arguments)
	set(derparguments "${arguments}")
	set(arguments ", ${arguments}")
	set(values ", ${values}")
  endif()
  m4(db/restartable_transaction.m4.c
	DEST "${target}.c"
	OPTIONS
	"-DFUNCTION_NAME=${function}"
	"-DARGUMENTS=${arguments}"
	"-DVALUES=${values}")
  add_custom_target("${target}"
	DEPENDS "${target}.c")
endfunction()
