# Find lcov, for obtaining code coverage.
#
# Defines LCOV_FOUND (bool) and LCOV (path to lcov executable).

find_program(LCOV lcov)
if(LCOV)
	set(LCOV_FOUND true)
	message(STATUS "Found lcov: ${LCOV}")
else()
	set(_message "Could NOT find lcov: Command not in PATH")
	if(Lcov_FIND_REQUIRED)
		message(FATAL_ERROR "${_message}")
	elseif(NOT Lcov_FIND_QUIETLY)
		message(STATUS "${_message}")
	endif()
endif()
