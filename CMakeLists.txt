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

message(STATUS "building ${plugin_name} as a plugin")

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/types SRC_TYPES)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/types
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/
)

# create a dynamic library
add_library(librime-qjs-objs OBJECT ${SRC} ${SRC_TYPES})

# Link QuickJS to librime-qjs
target_link_libraries(librime-qjs-objs PRIVATE quickjs)

##### testing #####
enable_testing()

# Set output directory for test executables
set(TEST_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)
file(MAKE_DIRECTORY ${TEST_OUTPUT_DIR})

# Find all quickjs test files automatically
file(GLOB QJS_TEST_FILES "tests/qjs/*.test.cpp")

# Create test targets
foreach(test_file ${QJS_TEST_FILES})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_name "${test_name}-test")
    add_executable(${test_name} ${test_file})
    set_target_properties(${test_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIR})
    target_link_libraries(${test_name} gtest gtest_main quickjs glog)
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()

# Find all quickjs test files automatically
file(GLOB QJS_GEAR_TEST_FILES "tests/*.test.cpp")

# Create test targets
foreach(test_file ${QJS_GEAR_TEST_FILES})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(test_name "${test_name}-test")
    add_executable(${test_name} ${test_file})
    set_target_properties(${test_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIR})
    target_link_libraries(${test_name}
        gtest
        gtest_main
        ${rime_library}
        ${rime_gears_library}
        quickjs
        librime-qjs-objs
    )
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
