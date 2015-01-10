cmake_minimum_required (VERSION 2.6.0)

project (enlighten)

### Generate a useful .ycm_extra_conf file
set (CMAKE_EXPORT_COMPILE_COMMANDS 1)
file (READ .ycm_extra_conf.py YCM_EXTRA_CONF_CONTENTS)
string (REGEX REPLACE "compilation_database_folder = '.*'(\r?\n)"
	"compilation_database_folder = '${CMAKE_BINARY_DIR}'\\1"
	UPDATED_YCM_EXTRA_CONF_CONTENTS "${YCM_EXTRA_CONF_CONTENTS}")
file (WRITE .ycm_extra_conf.py ${UPDATED_YCM_EXTRA_CONF_CONTENTS})

### Global options and flags
include_directories (
	include
	thirdparty/sqlite3/include)

list (APPEND CMAKE_CXX_FLAGS "-std=c++11")

### Include files
set (LIB_INCLUDE
	include/lrprev.h
	include/logger.h
	include/previewsdatabase.h
	include/previewentry.h
	include/validation.h)

### Source files
set (SQLITE_SOURCE
	thirdparty/sqlite3/src/sqlite3.c)
set (LIB_SOURCE
	src/lrprev.cpp
	src/logger.cpp
	src/previewsdatabase.cpp
	src/previewentry.cpp)
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

### Tests. Note that we are NOT using CMake's add_test() functionality. Instead
#   we define a custom unit_test target and rely on that to run our gtest tests.
add_subdirectory (thirdparty/gmock)
include_directories (
	thirdparty/gmock/include
	thirdparty/gtest/include)

# Lib tests
file (GLOB LIB_TEST_SOURCES test/spec/*_test.cpp)
set (LIB_UNIT_TEST_NAME     enlighten_lib_unit_tests)
add_executable (${LIB_UNIT_TEST_NAME} ${LIB_TEST_SOURCES})
target_link_libraries (${LIB_UNIT_TEST_NAME} enlighten_lib gmock_main)

# A target to run all unit tests. Add more tests to this target if needed.
add_custom_target (unit_tests
	COMMAND ${LIB_UNIT_TEST_NAME} --gtest_color=yes
	DEPENDS ${LIB_UNIT_TEST_NAME}
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test/data)
