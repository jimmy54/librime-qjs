#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <cstdio>
#include "trie_ext.hpp"
#include "trie_data_helper.h"

// Foreground colors
constexpr const char* RESET     = "\033[0m";
constexpr const char* BLACK     = "\033[30m";
constexpr const char* RED       = "\033[31m";
constexpr const char* GREEN     = "\033[32m";
constexpr const char* YELLOW    = "\033[33m";
constexpr const char* BLUE      = "\033[34m";
constexpr const char* MAGENTA   = "\033[35m";
constexpr const char* CYAN      = "\033[36m";
constexpr const char* WHITE     = "\033[37m";

// #define RUN_BENCHMARK_WITH_REAL_DATA

#ifdef RUN_BENCHMARK_WITH_REAL_DATA

TrieDataHelper trieDataHelper_ =
  TrieDataHelper("./tests/benchmark/dict", nullptr);

#else

TrieDataHelper trieDataHelper_ =
  TrieDataHelper("./tests/benchmark/dict", "dummy_dict.txt");

class GlobalEnvironment : public testing::Environment {
public:
  void SetUp() override {
    trieDataHelper_.createDummyTextFile();
  }

  void TearDown() override {
    trieDataHelper_.cleanupDummyFiles();
  }
};

#endif // RUN_BENCHMARK_WITH_REAL_DATA

void TestSearchItems(TrieWithStringExt& trie) {
  // Test lookup for existing words
  auto result1 = trie.find("accord");
  ASSERT_TRUE(result1.has_value());
  EXPECT_EQ(result1.value(), "[ә'kɒ:d]; n. 一致, 调和, 协定\\n vt. 给与, 使一致\\n vi. 相符合");

  auto result2 = trie.find("accordion");
  ASSERT_TRUE(result2.has_value());
  EXPECT_EQ(result2.value(), "[ә'kɒ:djәn]; n. 手风琴\\n a. 可折叠的");

  // Test lookup for non-existing word
  auto result3 = trie.find("nonexistent-word");
  ASSERT_FALSE(result3.has_value());

  // Test prefix search
  auto prefix_results = trie.prefix_search("accord");
  ASSERT_FALSE(prefix_results.empty());
  EXPECT_EQ(prefix_results.size(), 6); // All words starting with "accord"
}

TEST(LoadDictBenchmark, LoadTextFileAndLookup) {
  auto start = std::chrono::high_resolution_clock::now();

  TrieWithStringExt trie;
  trie.loadTextFile(trieDataHelper_.txtPath_, trieDataHelper_.entrySize_);

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  std::cout << MAGENTA
            << "Plain text file: \t\t"
            << duration.count()
            << " ms"
            << RESET
            << std::endl;

  trieDataHelper_.TestSearchItems(trie);

  // save the trie to a binary file for later tests
  std::ifstream file(trieDataHelper_.binaryPath_);
  if (!file.good()) {
      trie.save_to_files(trieDataHelper_.binaryPath_);
  }

  std::ifstream file2(trieDataHelper_.mergedBinaryPath_);
  if (!file2.good()) {
      trie.saveToBinaryFile(trieDataHelper_.mergedBinaryPath_);
  }
}

TEST(LoadDictBenchmark, LoadSingleBinaryFileWithMmapAndLookup) {
  auto start = std::chrono::high_resolution_clock::now();

  TrieWithStringExt trie;
  trie.loadBinaryFileMmap(trieDataHelper_.mergedBinaryPath_);

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  std::cout << MAGENTA
            << "Mmap single binary file: \t"
            << duration.count()
            << " ms"
            << RESET
            << std::endl;

  trieDataHelper_.TestSearchItems(trie);
}

TEST(LoadDictBenchmark, LoadSingleBinaryFileAndLookup) {
  auto start = std::chrono::high_resolution_clock::now();

  TrieWithStringExt trie;
  trie.load_from_single_file(trieDataHelper_.mergedBinaryPath_);

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  std::cout << MAGENTA
            << "Single binary file: \t\t"
            << duration.count()
            << " ms"
            << RESET
            << std::endl;

  trieDataHelper_.TestSearchItems(trie);
}


TEST(LoadDictBenchmark, LoadBinaryFileAndLookup) {
  auto start = std::chrono::high_resolution_clock::now();

  TrieWithStringExt trie;
  trie.load_from_files(trieDataHelper_.binaryPath_);

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  std::cout << MAGENTA
            << "Mutiple binary files: \t\t"
            << duration.count()
            << " ms"
            << RESET
            << std::endl;

  trieDataHelper_.TestSearchItems(trie);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

#ifndef RUN_BENCHMARK_WITH_REAL_DATA
  testing::AddGlobalTestEnvironment(new GlobalEnvironment);
#else
  // The real en2cn dictionary file: 3.8MB, 57614 rows
  trieDataHelper_.txtPath_ = "/Users/hj/Library/Rime/lua/data/ecdict.txt";
  trieDataHelper_.binaryPath_ = "./tests/benchmark/dict/dict.bin";
  trieDataHelper_.mergedBinaryPath_ = "./tests/benchmark/dict/merged-dict.bin";
  trieDataHelper_.entrySize_ = 60000;
#endif

  return RUN_ALL_TESTS();
}

// Benchmark of loading the dictionary:
// ----------------------------------
// Plain text file:           131 ms
// Mmap single binary file:    6 ms
// Single binary file:         13 ms
// Mutiple binary files:       11 ms
