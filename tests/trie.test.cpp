#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <cstdio>

#include "trie.h"
#include "trie_data_helper.h"

class TrieTest : public ::testing::Test {

protected:
  TrieDataHelper trieDataHelper_ = TrieDataHelper("./tests", "dummy_dict.txt");

  void SetUp() override {
      trieDataHelper_.createDummyTextFile();
    }

  void TearDown() override {
      trieDataHelper_.cleanupDummyFiles();
  }
};

TEST_F(TrieTest, LoadTextFileAndLookup) {
  rime::Trie trie;
  trie.loadTextFile(trieDataHelper_.txtPath_, trieDataHelper_.entrySize_);
  trieDataHelper_.TestSearchItems(trie);

  // save to file and load it back
  trie.saveToBinaryFile(trieDataHelper_.mergedBinaryPath_);
  rime::Trie trie2;
  trie2.loadBinaryFileMmap(trieDataHelper_.mergedBinaryPath_);
  trieDataHelper_.TestSearchItems(trie2);
}
