# CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(LibrimeQjs VERSION 1.0)

# Specify C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

set(GTEST_VERSION       "1.15.2")

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

set(CMAKE_CURRENT_LIST_DIR ${CMAKE_SOURCE_DIR}/CMakeFiles/list)
set(CMAKE_CURRENT_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)

# # Download and unpack Google Test
# include(FetchContent)
# FetchContent_Declare(
#     googletest
#     URL "https://github.com/google/googletest/archive/refs/tags/v${GTEST_VERSION}.tar.gz"
#     DOWNLOAD_EXTRACT_TIMESTAMP TRUE
# )
# # For Windows: Prevent overriding the parent project's compiler/linker settings
# set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(googletest)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    message(STATUS "building librime-qjs standalone")

    # Only include quickjs tests if we're building standalone (no parent scope)
    enable_testing()

    add_executable(ImportJsModuleTest tests/qjs/import-js-module.test.cpp)
    target_link_libraries(ImportJsModuleTest gtest gtest_main quickjs)
    add_test(NAME ImportJsModuleTest COMMAND ImportJsModuleTest)

    add_executable(ExposeCppClassTest tests/qjs/expose-cpp-class.test.cpp)
    target_link_libraries(ExposeCppClassTest gtest gtest_main quickjs)
    add_test(NAME ExposeCppClassTest COMMAND ExposeCppClassTest)

    add_executable(ReportJsErrorTest tests/qjs/report-js-error.test.cpp)
    target_link_libraries(ReportJsErrorTest gtest gtest_main quickjs)
    add_test(NAME ReportJsErrorTest COMMAND ReportJsErrorTest)
else()
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
    )

    # create a dynamic library
    add_library(librime-qjs-objs OBJECT ${SRC} ${SRC_TYPES})

    # Link QuickJS to librime-qjs
    target_link_libraries(librime-qjs-objs PRIVATE quickjs)
    # set_target_properties(librime-qjs-objs PROPERTIES POSITION_INDEPENDENT_CODE ON)

    message(STATUS "skip quickjs tests when building ${plugin_name} as a plugin")
endif()
