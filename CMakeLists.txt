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

# Overpass uses boost for parsing command-line parameters.
find_package(Boost REQUIRED COMPONENTS
	program_options
)

# Overpass uses libtins for packet processing.
find_package(libtins REQUIRED)

include_directories(
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_BINARY_DIR}/include
	${Boost_INCLUDE_DIRS}
	${LIBTINS_INCLUDE_DIRS}
)

set(OVERPASS_HEADERS
)

set(OVERPASS_SOURCES
	${PROJECT_SOURCE_DIR}/src/version.cpp
)

add_definitions("-std=c++11 -Wall -Wextra")

add_library(overpass
	${OVERPASS_HEADERS}
	${OVERPASS_SOURCES}
)

target_link_libraries(overpass
	${LIBTINS_LIBRARIES}
)

add_executable(overpassd
	${PROJECT_SOURCE_DIR}/src/main.cpp
)

target_link_libraries(overpassd
	overpass
	${Boost_LIBRARIES}
)

set(BUILD_TESTS true CACHE BOOL "Whether or not to build the tests")
if(BUILD_TESTS)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0") # Debug, no optimization
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage") # Enable coverage
	add_subdirectory(tests)
endif()
