set(FETCHCONTENT_QUIET OFF)
include(FetchContent)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../build)

# Google Benchmark
# https://github.com/ashvardanian/less_slow.cpp/blob/main/CMakeLists.txt
FetchContent_Declare(
  GoogleBenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.9.1
)
# Suppress building tests/docs/etc. for faster builds:
set(BENCHMARK_ENABLE_TESTING
  OFF
  CACHE BOOL "" FORCE
)
set(BENCHMARK_ENABLE_INSTALL
  OFF
  CACHE BOOL "" FORCE
)
set(BENCHMARK_ENABLE_DOXYGEN
  OFF
  CACHE BOOL "" FORCE
)
set(BENCHMARK_INSTALL_DOCS
  OFF
  CACHE BOOL "" FORCE
)
set(BENCHMARK_DOWNLOAD_DEPENDENCIES
  ON
  CACHE BOOL "" FORCE
)
set(BENCHMARK_ENABLE_GTEST_TESTS
  OFF
  CACHE BOOL "" FORCE
)
set(BENCHMARK_USE_BUNDLED_GTEST
  ON
  CACHE BOOL "" FORCE
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(BENCHMARK_ENABLE_LIBPFM
      OFF
      CACHE BOOL "" FORCE
  )
endif ()

FetchContent_MakeAvailable(GoogleBenchmark)
include_directories(${GoogleBenchmark}/Headers)

# Remove Google Benchmark's built-in debug warning in Release mode:
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(benchmark PRIVATE NDEBUG)
endif ()

# aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/dict SRC_DICT)
set(SRC_DICT "dict/dictionary_benchmark.cc")

add_executable(qjs-benchmark ${SRC_DICT})
target_link_libraries(qjs-benchmark
  librime-qjs-objs
  ${rime_library}
  ${rime_dict_library}
  ${rime_gears_library}
  benchmark
)
