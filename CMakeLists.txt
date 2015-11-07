cmake_minimum_required(VERSION 2.6)
project(liboverpass)

# Version number
set(VERSION_MAJOR 0)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)

configure_file(
	${PROJECT_SOURCE_DIR}/include/version.h.in
	${PROJECT_BINARY_DIR}/include/version.h
)

include_directories(${PROJECT_BINARY_DIR}/include)
