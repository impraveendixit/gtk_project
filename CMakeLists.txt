# Specify the minimum version for CMake
cmake_minimum_required(VERSION 3.0)

# Project's name
project(Drinks VERSION 1.0)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

# json-c' library
find_path(JSONC_INCLUDE_DIR json-c/json.h)
find_library(JSONC_LIBRARIES NAMES json-c)

# libcurl
pkg_check_modules(CURL libcurl REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

include_directories(${GTK3_INCLUDE_DIRS})
include_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

include_directories("${PROJECT_SOURCE_DIR}")

# Set the output folder where your program will be created
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(SOURCES main.c cocktail.c ui.c)

# The following folder will be included
add_executable(drinks ${SOURCES})
target_link_libraries(drinks ${GTK3_LIBRARIES} ${JSONC_LIBRARIES} ${CURL_LIBRARIES})
