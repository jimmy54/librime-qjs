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

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC)

set(CMAKE_CURRENT_LIST_DIR ${CMAKE_SOURCE_DIR}/CMakeFiles/list)
set(CMAKE_CURRENT_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)

set(SOURCES ${SRC})

# create a dynamic library
message(STATUS "Plugin ${plugin_name} src: ${SOURCES}")
add_library(librime-qjs-objs OBJECT ${SOURCES})

# Link QuickJS to librime-qjs
target_link_libraries(librime-qjs-objs PRIVATE quickjs)

# rime plugin
set(plugin_name "rime-qjs" PARENT_SCOPE)
set(plugin_objs $<TARGET_OBJECTS:librime-qjs-objs> PARENT_SCOPE)
set(plugin_deps ${rime_library} ${rime_gears_library} PARENT_SCOPE)
set(plugin_modules "qjs" PARENT_SCOPE)

# # Download and unpack Google Test
# include(FetchContent)
# FetchContent_Declare(
#   googletest
#   URL "https://github.com/google/googletest/archive/refs/tags/v${GTEST_VERSION}.tar.gz"
#   DOWNLOAD_EXTRACT_TIMESTAMP TRUE
# )
# # For Windows: Prevent overriding the parent project's compiler/linker settings
# set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(googletest)

# include_directories(include ${CMAKE_CURRENT_SOURCE_DIR}/src)

# # Add tests
# add_executable(ImportJsModuleTest tests/qjs/import-js-module.test.cpp)
# target_link_libraries(ImportJsModuleTest gtest gtest_main quickjs)

# add_executable(ExposeCppClassTest tests/qjs/expose-cpp-class.test.cpp)
# target_link_libraries(ExposeCppClassTest gtest gtest_main quickjs)
