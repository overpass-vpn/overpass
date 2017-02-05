add_executable(unit-tests
	${PROJECT_SOURCE_DIR}/tests/unit/src/main.cpp
	${PROJECT_SOURCE_DIR}/tests/unit/src/test_datagram_server.cpp
	${PROJECT_SOURCE_DIR}/tests/unit/src/test_router.cpp
	${PROJECT_SOURCE_DIR}/tests/unit/src/test_stream_server.cpp
	${PROJECT_SOURCE_DIR}/tests/unit/src/test_version.cpp
)

target_link_libraries(unit-tests
	overpass
	gmock
	gmock_main
	${Boost_LIBRARIES}
)
