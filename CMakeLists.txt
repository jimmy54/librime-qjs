# CMakeLists.txt

cmake_minimum_required(VERSION 3.12)

# Enable compilation database for clangd and clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

### Disable clang-tidy during build, since it reports plenty of unrelated issues outside our codebase
# set(CMAKE_CXX_CLANG_TIDY "/opt/local/libexec/llvm-20/bin/clang-tidy")

# Set the project name and version
project(LibrimeQjs VERSION 0.1)

add_definitions(-DRIME_QJS_VERSION="${PROJECT_VERSION}")

# Specify C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add _GNU_SOURCE definition for Boost.Stacktrace
add_definitions(-D_GNU_SOURCE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/quickjs/quickjs.h")
  # QuickJS source files
  set(QUICKJS_SOURCES
      ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/quickjs/quickjs.c
      ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/quickjs/libregexp.c
      ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/quickjs/libunicode.c
      ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/quickjs/libbf.c
      ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/quickjs/cutils.c
  )

  include_directories(include ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/quickjs)

  # Create QuickJS library
  add_library(qjs STATIC ${QUICKJS_SOURCES})
  target_include_directories(qjs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/quickjs)
else()
  find_package(quickjs REQUIRED CONFIGS qjsConfig.cmake)
endif()

set(CMAKE_CURRENT_LIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/list)
set(CMAKE_CURRENT_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)

set(plugin_name "rime-qjs" PARENT_SCOPE)
set(plugin_objs $<TARGET_OBJECTS:librime-qjs-objs> PARENT_SCOPE)
set(plugin_deps ${rime_library} ${rime_gears_library} qjs PARENT_SCOPE)
set(plugin_modules "qjs" PARENT_SCOPE)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/gears SRC_GEARS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/types SRC_TYPES)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/helpers SRC_HELPERS)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/yas/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gears
    ${CMAKE_CURRENT_SOURCE_DIR}/src/types
    ${CMAKE_CURRENT_SOURCE_DIR}/src/helpers
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/
)

# create a dynamic library
add_library(librime-qjs-objs OBJECT ${SRC} ${SRC_GEARS} ${SRC_TYPES} ${SRC_HELPERS})
if(BUILD_SHARED_LIBS)
  set_target_properties(librime-qjs-objs PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# Link QuickJS to librime-qjs
target_link_libraries(librime-qjs-objs PRIVATE qjs)

##### testing #####
enable_testing()

# Set output directory for test executables
set(TEST_OUTPUT_DIR ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

file(GLOB QJS_TEST_FILES "tests/qjs/*.test.cpp")
file(GLOB QJS_GEAR_TEST_FILES "tests/*.test.cpp")
add_executable(librime-qjs-tests ${QJS_TEST_FILES} ${QJS_GEAR_TEST_FILES})
target_link_libraries(librime-qjs-tests
  ${rime_library}
  ${rime_dict_library}
  ${rime_gears_library}
  ${GTEST_LIBRARIES}
  qjs
  librime-qjs-objs
)

add_test(NAME librime-qjs-tests
  COMMAND librime-qjs-tests
  WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})

# set(benchmark_files "tests/benchmark/dict/load_trie_benchmark.cc")
# add_executable(load-dict-benchmark ${benchmark_files})
# target_link_libraries(load-dict-benchmark
#   ${rime_library}
#   ${rime_dict_library}
#   ${rime_gears_library}
#   ${GTEST_LIBRARIES}
# )
