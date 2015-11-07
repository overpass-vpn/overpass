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

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_BINARY_DIR})

find_package(Boost REQUIRED COMPONENTS
	system
	program_options
)
include_directories(${Boost_INCLUDE_DIRS})

set(OVERPASS_HEADERS
	${PROJECT_SOURCE_DIR}/include/virtual_interface.h
)

set(OVERPASS_SOURCES
	${PROJECT_SOURCE_DIR}/src/virtual_interface.cpp
	${PROJECT_SOURCE_DIR}/src/main.cpp
)

add_definitions("-std=c++11 -Wall -Wextra")

add_executable(overpass
	${OVERPASS_HEADERS}
	${OVERPASS_SOURCES}
)

target_link_libraries(overpass ${Boost_LIBRARIES} pthread)
