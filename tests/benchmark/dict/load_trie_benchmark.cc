#include <gtest/gtest.h>

#include <string>

#include "benchmark_helper.h"
#include "trie_data_helper.h"
#include "trie_ext.hpp"

// #define RUN_BENCHMARK_WITH_REAL_DATA

#ifdef RUN_BENCHMARK_WITH_REAL_DATA

TrieDataHelper trieDataHelper_ = TrieDataHelper("./tests/benchmark/dict", nullptr);

#else

const TrieDataHelper TRIE_DATA_HELPER = TrieDataHelper("./tests/benchmark/dict", "dummy_dict.txt");

class GlobalEnvironment : public testing::Environment {
public:
  void SetUp() override { TRIE_DATA_HELPER.createDummyTextFile(); }

  void TearDown() override { TRIE_DATA_HELPER.cleanupDummyFiles(); }
};

#endif  // RUN_BENCHMARK_WITH_REAL_DATA

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void testSearchItems(TrieWithStringExt& trie) {
  // Test lookup for existing words
  auto result1 = trie.find("accord");
  ASSERT_TRUE(result1.has_value());
  if (result1.has_value()) {
    EXPECT_EQ(result1.value(), "[ә'kɒ:d]; n. 一致, 调和, 协定\\n vt. 给与, 使一致\\n vi. 相符合");
  }

  auto result2 = trie.find("accordion");
  ASSERT_TRUE(result2.has_value());
  if (result2.has_value()) {
    EXPECT_EQ(result2.value(), "[ә'kɒ:djәn]; n. 手风琴\\n a. 可折叠的");
  }

  // Test lookup for non-existing word
  auto result3 = trie.find("nonexistent-word");
  ASSERT_FALSE(result3.has_value());

  // Test prefix search
  auto prefixResults = trie.prefixSearch("accord");
  ASSERT_FALSE(prefixResults.empty());
  EXPECT_EQ(prefixResults.size(), 6);  // All words starting with "accord"
}

TEST(LoadTrieDictBenchmark, LoadTextFileAndLookup) {
  TrieWithStringExt trie;
  PRINT_DURATION(MAGENTA, "Plain text file: \t\t\t",
                 trie.loadTextFile(TRIE_DATA_HELPER.txtPath_, TRIE_DATA_HELPER.entrySize_));
  TrieDataHelper::testSearchItems(trie);

  RESAVE_FILE(TRIE_DATA_HELPER.binaryPath_,
              PRINT_DURATION(YELLOW, "mmap trie + r/w data as string: \t",
                             trie.saveToFiles(TRIE_DATA_HELPER.binaryPath_)))

  RESAVE_FILE(TRIE_DATA_HELPER.mergedBinaryPath_,
              PRINT_DURATION(YELLOW, "mmap (trie + data): \t\t\t",
                             trie.saveToBinaryFile(TRIE_DATA_HELPER.mergedBinaryPath_)))

  auto yasPath = TRIE_DATA_HELPER.mergedBinaryPath_ + ".yas";
  RESAVE_FILE(yasPath, PRINT_DURATION(YELLOW, "YAS  (trie + data): \t\t\t",
                                      trie.saveToBinaryFileYas(yasPath)))
}

TEST(LoadTrieDictBenchmark, LoadSingleBinaryFileWithMmapAndLookup) {
  TrieWithStringExt trie;
  PRINT_DURATION(MAGENTA, "mmap (trie + data): \t\t\t",
                 trie.loadBinaryFileMmap(TRIE_DATA_HELPER.mergedBinaryPath_));
  TrieDataHelper::testSearchItems(trie);
}

TEST(LoadTrieDictBenchmark, LoadSingleBinaryFileAndLookup) {
  TrieWithStringExt trie;
  PRINT_DURATION(MAGENTA, "mmap trie + r/w data as vector: \t",
                 trie.loadFromSingleFile(TRIE_DATA_HELPER.mergedBinaryPath_));
  TrieDataHelper::testSearchItems(trie);
}

TEST(LoadTrieDictBenchmark, LoadBinaryFilesAndLookup) {
  TrieWithStringExt trie;
  PRINT_DURATION(MAGENTA, "mmap trie + r/w data as string: \t",
                 trie.loadFromFiles(TRIE_DATA_HELPER.binaryPath_));
  TrieDataHelper::testSearchItems(trie);
}

TEST(LoadTrieDictBenchmark, LoadSingleBinaryFileAndLookupYas) {
  TrieWithStringExt trie;
  auto yasPath = TRIE_DATA_HELPER.mergedBinaryPath_ + ".yas";
  PRINT_DURATION(MAGENTA, "YAS  (trie + data): \t\t\t", trie.loadBinaryFileYas(yasPath));
  TrieDataHelper::testSearchItems(trie);
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);

#ifndef RUN_BENCHMARK_WITH_REAL_DATA
  testing::AddGlobalTestEnvironment(new GlobalEnvironment);
#else
  // The real en2cn dictionary file: 3.8MB, 57614 rows
  TrieDataHelper.txtPath_ = "/Users/hj/Library/Rime/lua/data/ecdict.txt";
  TrieDataHelper.entrySize_ = 60000;

  TrieDataHelper.binaryPath_ = "./tests/benchmark/dict/dict.bin";
  TrieDataHelper.mergedBinaryPath_ = "./tests/benchmark/dict/merged-dict.bin";
#endif

  return RUN_ALL_TESTS();
}

// Benchmark of loading the en2cn dictionary (3.8MB, 57614 rows)
// +=======================================================================================================================+
// | Option                         | Load from Text | Save to Binary | Load from Binary |  Hardware                       |
// +--------------------------------+----------------+----------------+------------------+---------------------------------+
// | mmap (trie + data)             |  110 ms        |  24 ms         |   5 ms           | MBP 2015, 2.4 GHz Intel Core i7 |
// | mmap trie + r/w data as vector |  110 ms        |  24 ms         |  13 ms           | MBP 2015, 2.4 GHz Intel Core i7 |
// | mmap trie + r/w data as string |  110 ms        |  22 ms         |   8 ms           | MBP 2015, 2.4 GHz Intel Core i7 |
// | YAS  (trie + data)             |  110 ms        |  26 ms         |   8 ms           | MBP 2015, 2.4 GHz Intel Core i7 |
// +=======================================================================================================================+
