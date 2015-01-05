cmake_minimum_required (VERSION 2.6.0)

project (enlighten)

### Global options and flags
include_directories (
	include
	thirdparty/sqlite3/include)

### Include files
set (LIB_INCLUDE
	include/lrprev.h
	include/validation.h)

### Source files
set (SQLITE_SOURCE
	thirdparty/sqlite3/src/sqlite3.c)
set (LIB_SOURCE
	src/lrprev.cpp)
set (CLI_SOURCE
	src/cli.cpp)

### Executables and Targets
# Core functionality library
add_library (enlighten_lib
	STATIC
	${LIB_SOURCE}
	${LIB_INCLUDE}
	${SQLITE_SOURCE})

# CLI executable
add_executable (enlighten_cli
	${CLI_SOURCE})
target_link_libraries (enlighten_cli enlighten_lib)

### Tests
add_subdirectory (thirdparty/gmock)
include_directories (
	thirdparty/gmock/include
	thirdparty/gtest/include)

file (GLOB LIB_TESTS test/*_test.cpp)

add_executable (enlighten_tests ${LIB_TESTS})
target_link_libraries (enlighten_tests enlighten_lib gmock_main)

