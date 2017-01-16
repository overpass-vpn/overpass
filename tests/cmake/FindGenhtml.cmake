# Find genhtml, for generating an HTML coverage report.
#
# Defines GENHTML_FOUND (bool) and GENHTML (path to genhtml executable).

find_program(GENHTML genhtml)
if(GENHTML)
	set(GENHTML_FOUND true)
	message(STATUS "Found genhtml: ${GENHTML}")
else()
	set(_message "Could NOT find genhtml: Command not in PATH")
	if(Genhtml_FIND_REQUIRED)
		message(FATAL_ERROR "${_message}")
	elseif(NOT Genhtml_FIND_QUIETLY)
		message(STATUS "${_message}")
	endif()
endif()
