# Build with system gmock and embedded gtest
set(GMOCK_INCLUDE_DIR "/usr/include/gmock/include" CACHE PATH "gmock source include directory")
set(GMOCK_SOURCE_DIR "/usr/src/gmock" CACHE PATH "gmock source directory")
set(GTEST_INCLUDE_DIR "${GMOCK_SOURCE_DIR}/gtest/include" CACHE PATH "gtest source include directory")

add_subdirectory(${GMOCK_SOURCE_DIR} "${CMAKE_CURRENT_BINARY_DIR}/gmock")
include_directories(
	${GTEST_INCLUDE_DIR}
	${GMOCK_INCLUDE_DIR}
)

find_program(LCOV lcov)
if(NOT LCOV)
	message(FATAL_ERROR "lcov needs to be in the PATH for coverage generation.")
endif()

# Generate the initial coverage (so we can get uncovered files)
add_custom_command(COMMENT "Generating initial coverage..."
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
	OUTPUT ${PROJECT_BINARY_DIR}/initial.coverage
	COMMAND ${LCOV} -q -c -i -d . -o initial.coverage
)

# Generate the tested coverage (so we get real coverage)
add_custom_command(COMMENT "Generating tested coverage..."
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
	OUTPUT ${PROJECT_BINARY_DIR}/tested.coverage
	COMMAND ${LCOV} -q -c -d . -o tested.coverage
	DEPENDS test
)

# Merge the initial and tested coverage and filter the results
add_custom_command(COMMENT "Generating final coverage..."
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
	OUTPUT ${PROJECT_BINARY_DIR}/final.coverage
	COMMAND ${LCOV} -q -a initial.coverage -a tested.coverage -o total.coverage
	COMMAND ${LCOV} -q -r total.coverage 'tests/*' '/usr/*' -o final.coverage
	DEPENDS ${PROJECT_BINARY_DIR}/initial.coverage ${PROJECT_BINARY_DIR}/tested.coverage
)

# Display the final coverage results
add_custom_target(coverage
	COMMENT "Parsing final coverage report..."
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
	COMMAND ${LCOV} --summary final.coverage
	COMMAND ${LCOV} --list final.coverage
	DEPENDS ${PROJECT_BINARY_DIR}/final.coverage
)

# Actually run tests. Add any new test executables as new COMMANDS here.
add_custom_target(test
	COMMENT "Running tests..."
	COMMAND unit-tests
	DEPENDS ${PROJECT_BINARY_DIR}/initial.coverage
)

add_subdirectory(unit)
