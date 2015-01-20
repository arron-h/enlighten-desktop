cmake_minimum_required (VERSION 2.6.0)

project (enlighten)

### Generate a useful .ycm_extra_conf file
if (${CMAKE_GENERATOR} MATCHES "Unix Makefiles")
	message ("Generating compile commands")
	set (CMAKE_EXPORT_COMPILE_COMMANDS 1)
	file (READ .ycm_extra_conf.py YCM_EXTRA_CONF_CONTENTS)
	string (REGEX REPLACE "compilation_database_folder = '.*'(\r?\n)"
		"compilation_database_folder = '${CMAKE_BINARY_DIR}'\\1"
		UPDATED_YCM_EXTRA_CONF_CONTENTS "${YCM_EXTRA_CONF_CONTENTS}")
	file (WRITE .ycm_extra_conf.py ${UPDATED_YCM_EXTRA_CONF_CONTENTS})
endif ()

### Dependencies
find_package (JPEG REQUIRED)
list (APPEND DEPENDENCY_LIBS ${JPEG_LIBRARIES})

### Global options and flags
include_directories (
	include
	thirdparty/sqlite3/include
	${JPEG_INCLUDE_DIR})

list (APPEND CMAKE_CXX_FLAGS "-std=c++11")
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set (PLATFORM_LINK_LIBS "dl")
endif ()

### Include files
set (LIB_INCLUDE
	include/cachedpreviews.h
	include/ifile.h
	include/jpeg.h
	include/jpegcruncher.h
	include/lrprev.h
	include/logger.h
	include/previewsdatabase.h
	include/previewentry.h
	include/previewentrylevel.h
	include/settings.h
	include/syncaction.h
	include/validation.h
	include/watcher.h
	include/synchronizers/abstractsynchronizer.h
	include/synchronizers/previewssynchronizer.h)

### Source files
set (SQLITE_SOURCE
	thirdparty/sqlite3/src/sqlite3.c)
set (LIB_SOURCE
	src/aws.cpp
	src/awsrequest.cpp
	src/cachedpreviews.cpp
	src/jpeg.cpp
	src/jpegcruncher.cpp
	src/lrprev.cpp
	src/logger.cpp
	src/previewsdatabase.cpp
	src/previewentry.cpp
	src/previewentrylevel.cpp
	src/settings.cpp
	src/watcher.cpp
	src/synchronizers/previewssynchronizer.cpp)
set (CLI_SOURCE
	src/cli.cpp)

### Platform source/include files
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set (PLATFORM_LIB_INCLUDE
		include/platform/osx/file.h)
	set (PLATFORM_LIB_SOURCE
		src/platform/osx/file.cpp)

	include_directories(include/platform/osx)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set (PLATFORM_LIB_INCLUDE
		include/platform/linux/file.h)
	set (PLATFORM_LIB_SOURCE
		src/platform/linux/file.cpp)

	include_directories(include/platform/linux)
endif ()

### Executables and Targets
# Core functionality library
add_library (enlighten_lib
	STATIC
	${LIB_SOURCE}
	${LIB_INCLUDE}
	${PLATFORM_LIB_SOURCE}
	${PLATFORM_LIB_INCLUDE}
	${SQLITE_SOURCE})

# CLI executable
add_executable (enlighten_cli
	${CLI_SOURCE})
target_link_libraries (enlighten_cli enlighten_lib ${PLATFORM_LINK_LIBS} ${DEPENDENCY_LIBS})

### Tests. Note that we are NOT using CMake's add_test() functionality. Instead
#   we define a custom unit_test target and rely on that to run our gtest tests.
add_subdirectory (thirdparty/gmock)
include_directories (
	thirdparty/gmock/include
	thirdparty/gtest/include)

# Lib unit tests
file (GLOB LIB_TEST_SOURCES test/unit/lib/*_test.cpp)
list (APPEND LIB_TEST_SOURCES test/libunittests.cpp)
set (LIB_UNIT_TEST_NAME     enlighten_lib_unit_tests)
add_executable (${LIB_UNIT_TEST_NAME} ${LIB_TEST_SOURCES})
target_link_libraries (${LIB_UNIT_TEST_NAME} enlighten_lib gmock ${PLATFORM_LINK_LIBS} ${DEPENDENCY_LIBS})

# Lib integration tests
file (GLOB LIB_INTEGRATION_TEST_SOURCES test/integration/lib/*_test.cpp)
list (APPEND LIB_INTEGRATION_TEST_SOURCES test/libintegrationtests.cpp)
set (LIB_INTEGRATION_TEST_NAME     enlighten_lib_integration_tests)
add_executable (${LIB_INTEGRATION_TEST_NAME} ${LIB_INTEGRATION_TEST_SOURCES})
target_link_libraries (${LIB_INTEGRATION_TEST_NAME} enlighten_lib gmock ${PLATFORM_LINK_LIBS} ${DEPENDENCY_LIBS})

# A target to run all unit tests. Add more tests to this target if needed.
add_custom_target (unit_tests
	COMMAND ${LIB_UNIT_TEST_NAME} --gtest_color=yes
	DEPENDS ${LIB_UNIT_TEST_NAME}
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test/data)

add_custom_target (integration_tests
	COMMAND ${LIB_INTEGRATION_TEST_NAME} --gtest_color=yes
	DEPENDS ${LIB_INTEGRATION_TEST_NAME}
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test/data)
