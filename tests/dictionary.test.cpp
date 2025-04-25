#include <gtest/gtest.h>
#include <memory>

#include "dict_data_helper.hpp"
#include "dicts/leveldb.hpp"
#include "dicts/trie.h"

#include "test_helper.hpp"

class DictionaryTest : public ::testing::Test {
private:
  DictionaryDataHelper dictHelper_ =
      DictionaryDataHelper(getFolderPath(__FILE__).c_str(), "dummy_dict.txt");

protected:
  DictionaryDataHelper getDictHelper() { return dictHelper_; }
  void SetUp() override { dictHelper_.createDummyTextFile(); }

  void TearDown() override { dictHelper_.cleanupDummyFiles(); }
};

TEST_F(DictionaryTest, LoadTextFileAndLookupWithTrie) {
  rime::Trie trie;
  auto helper = getDictHelper();
  ParseTextFileOptions options;
  options.lines = helper.entrySize_;
  trie.loadTextFile(helper.txtPath_, options);
  DictionaryDataHelper::testSearchItems(trie);

  // save to file and load it back
  trie.saveToBinaryFile(helper.mergedBinaryPath_);
  rime::Trie trie2;
  trie2.loadBinaryFile(helper.mergedBinaryPath_);
  DictionaryDataHelper::testSearchItems(trie2);
}

TEST_F(DictionaryTest, LoadTextFileAndLookupWithLevelDb) {
  auto helper = getDictHelper();

  LevelDb dict;
  ParseTextFileOptions options;
  options.lines = helper.entrySize_;
  dict.loadTextFile(helper.txtPath_, options);
  dict.saveToBinaryFile(helper.levelDbFolderPath_);
  DictionaryDataHelper::testSearchItems(dict);
  dict.close();  // a level db could be loaded only once

  LevelDb dict2;
  dict2.loadBinaryFile(helper.levelDbFolderPath_);
  DictionaryDataHelper::testSearchItems(dict2);
}
