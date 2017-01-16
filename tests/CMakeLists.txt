list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(Gmock REQUIRED)
include_directories(
	${GTEST_INCLUDE_DIRS}
	${GMOCK_INCLUDE_DIRS}
)

find_package(Lcov)
if(LCOV_FOUND)
	set(INITIAL_COVERAGE_FILE ${PROJECT_BINARY_DIR}/initial.coverage)
	set(TESTED_COVERAGE_FILE ${PROJECT_BINARY_DIR}/tested.coverage)
	set(FINAL_COVERAGE_FILE ${PROJECT_BINARY_DIR}/final.coverage)

	add_custom_target(initialize-coverage
		COMMENT "Initializing coverage..."
		WORKING_DIRECTORY ${PROJECT_BINARY_DIR}

		# Remove .gcda files from previous testing runs
		COMMAND ${LCOV} -d . -z -q

		# Generate initial coverage (so we can get uncovered files)
		COMMAND ${LCOV} -q -c -i -d . -o ${INITIAL_COVERAGE_FILE}
	)

   add_custom_target(finalize-coverage
		COMMENT "Calculating final coverage..."
		WORKING_DIRECTORY ${PROJECT_BINARY_DIR}

		# Generate coverage now that tests have run
		COMMAND ${LCOV} -q -c -d . -o ${TESTED_COVERAGE_FILE}

		# Combine and filter the initial and tested coverage reports
		COMMAND ${LCOV} -q -a ${INITIAL_COVERAGE_FILE} -a ${TESTED_COVERAGE_FILE} -o total.coverage
		COMMAND ${LCOV} -q -r total.coverage 'tests/*' '/usr/*' -o ${FINAL_COVERAGE_FILE}

		DEPENDS run-tests
	)

   # Display the final coverage results
	add_custom_target(coverage
		COMMENT "Parsing final coverage report..."
		WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
		COMMAND ${LCOV} --summary ${FINAL_COVERAGE_FILE}
		COMMAND ${LCOV} --list ${FINAL_COVERAGE_FILE}
		DEPENDS finalize-coverage
	)

	find_package(Genhtml)
	if(GENHTML_FOUND)
		# Generate an HTML coverage report
		add_custom_target(coverage-html
			COMMENT "Generating HTML coverage report..."
			WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
			COMMAND ${GENHTML} --legend --show-details -t "${PROJECT_NAME} test coverage" -o coverage-html ${FINAL_COVERAGE_FILE}
			DEPENDS coverage
		)
	else()
		message(WARNING "genhtml not found: HTML coverage report not available.")
		add_custom_target(coverage-html DEPENDS coverage)
	endif()
else()
	message(WARNING "lcov not found: coverage report not available.")
	add_custom_target(initialize-coverage)
	add_custom_target(coverage-html DEPENDS run-tests)
endif()

# Actually run tests. Add any new test executables as new COMMANDS here.
add_custom_target(run-tests
	COMMENT "Running tests..."
	COMMAND unit-tests
	DEPENDS initialize-coverage
)

add_custom_target(test
	DEPENDS coverage-html
)

add_subdirectory(unit)
