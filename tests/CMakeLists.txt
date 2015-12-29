list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(Gmock REQUIRED)
include_directories(
	${GTEST_INCLUDE_DIRS}
	${GMOCK_INCLUDE_DIRS}
)

find_package(Lcov)
if(LCOV_FOUND)
	set(INITIAL_COVERAGE_FILE ${PROJECT_BINARY_DIR}/initial.coverage)

	# Generate the initial coverage (so we can get uncovered files)
	add_custom_command(COMMENT "Generating initial coverage..."
		WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
		OUTPUT ${INITIAL_COVERAGE_FILE}
		COMMAND ${LCOV} -q -c -i -d . -o ${INITIAL_COVERAGE_FILE}
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

	find_package(Genhtml)
	if(GENHTML_FOUND)
		# Generate an HTML coverage report
		add_custom_target(coverage-html
			COMMENT "Generating HTML coverage report..."
			WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
			COMMAND ${GENHTML} --legend --show-details -t "${PROJECT_NAME} test coverage" -o coverage-html ${PROJECT_BINARY_DIR}/final.coverage
			DEPENDS ${PROJECT_BINARY_DIR}/final.coverage
		)
	else()
		message(WARNING "genhtml not found: HTML coverage report not available.")
	endif()
else()
	message(WARNING "lcov not found: coverage report not available.")
endif()

# Actually run tests. Add any new test executables as new COMMANDS here.
add_custom_target(test
	COMMENT "Running tests..."
	COMMAND unit-tests
	DEPENDS ${INITIAL_COVERAGE_FILE}
)

add_subdirectory(unit)
