# CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(LibrimeQjs VERSION 1.0)

# Specify C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add _GNU_SOURCE definition for Boost.Stacktrace
add_definitions(-D_GNU_SOURCE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)

# QuickJS source files
set(QUICKJS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/quickjs/quickjs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/quickjs/libregexp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/quickjs/libunicode.c
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/quickjs/libbf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/quickjs/cutils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/quickjs/quickjs-libc.c
)

include_directories(include ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/quickjs)

# Create QuickJS library
add_library(quickjs STATIC ${QUICKJS_SOURCES})
target_include_directories(quickjs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/quickjs)

set(CMAKE_CURRENT_LIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/list)
set(CMAKE_CURRENT_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)

set(plugin_name "rime-qjs" PARENT_SCOPE)
set(plugin_objs $<TARGET_OBJECTS:librime-qjs-objs> PARENT_SCOPE)
set(plugin_deps ${rime_library} ${rime_gears_library} quickjs PARENT_SCOPE)
set(plugin_modules "qjs" PARENT_SCOPE)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/types SRC_TYPES)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/helpers SRC_HELPERS)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/types
    ${CMAKE_CURRENT_SOURCE_DIR}/src/helpers
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/
)

# create a dynamic library
add_library(librime-qjs-objs OBJECT ${SRC} ${SRC_TYPES} ${SRC_HELPERS})

# Link QuickJS to librime-qjs
target_link_libraries(librime-qjs-objs PRIVATE quickjs)

##### testing #####
enable_testing()

# Set output directory for test executables
set(TEST_OUTPUT_DIR ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# Create a symbolic link for the YAML file
execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_CURRENT_SOURCE_DIR}/../../data/minimal/default.yaml
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/yaml/default.yaml
)

file(GLOB QJS_TEST_FILES "tests/qjs/*.test.cpp")
file(GLOB QJS_GEAR_TEST_FILES "tests/*.test.cpp")
aux_source_directory(tests/qjs TEST_SRC)
add_executable(librime-qjs-tests ${QJS_TEST_FILES} ${QJS_GEAR_TEST_FILES})
target_link_libraries(librime-qjs-tests
  ${rime_library}
  ${rime_dict_library}
  ${rime_gears_library}
  ${GTEST_LIBRARIES}
  quickjs
  librime-qjs-objs
)

add_test(NAME librime-qjs-tests
  COMMAND librime-qjs-tests
  WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})
