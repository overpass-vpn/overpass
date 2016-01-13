cmake_minimum_required(VERSION 2.6)
project(overpass)

# Version number
set(VERSION_MAJOR 0)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)

configure_file(
	${PROJECT_SOURCE_DIR}/data/version.h.in
	${PROJECT_BINARY_DIR}/include/version.h
)

include_directories(
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_BINARY_DIR}/include
)

set(OVERPASS_HEADERS
	${PROJECT_SOURCE_DIR}/include/udp_packet.h
)

set(OVERPASS_SOURCES
	${PROJECT_SOURCE_DIR}/src/udp_packet.cpp
)

add_definitions("-std=c++11 -Wall -Wextra")

add_library(overpass
	${OVERPASS_HEADERS}
	${OVERPASS_SOURCES}
)

set(BUILD_TESTS true CACHE BOOL "Whether or not to build the tests")
if(BUILD_TESTS)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0") # Debug, no optimization
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage") # Enable coverage
	add_subdirectory(tests)
endif()
