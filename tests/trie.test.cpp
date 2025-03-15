#include "trie.h"

#include <gtest/gtest.h>

#include "test_helper.h"
#include "trie_data_helper.h"

class TrieTest : public ::testing::Test {
private:
  TrieDataHelper trieDataHelper_ =
      TrieDataHelper(getFolderPath(__FILE__).c_str(), "dummy_dict.txt");

protected:
  TrieDataHelper getTrieDataHelper() { return trieDataHelper_; }
  void SetUp() override { trieDataHelper_.createDummyTextFile(); }

  void TearDown() override { trieDataHelper_.cleanupDummyFiles(); }
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, readability-function-cognitive-complexity)
TEST_F(TrieTest, LoadTextFileAndLookup) {
  rime::Trie trie;
  auto helper = getTrieDataHelper();
  trie.loadTextFile(helper.txtPath_, helper.entrySize_);
  TrieDataHelper::testSearchItems(trie);

  // save to file and load it back
  trie.saveToBinaryFile(helper.mergedBinaryPath_);
  rime::Trie trie2;
  trie2.loadBinaryFileMmap(helper.mergedBinaryPath_);
  TrieDataHelper::testSearchItems(trie2);
}
