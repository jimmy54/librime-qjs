#ifndef RIME_TRIEDATAHELPER_H_
#define RIME_TRIEDATAHELPER_H_

#include <iostream>
#include <fstream>
#include <string>
#include <gtest/gtest.h>
#include "trie.h"

class TrieDataHelper {
 public:
  TrieDataHelper(const char* folder, const char* dummyFileName) {
    if (dummyFileName == nullptr) {
      return;
    }

    txtPath_ = std::string(folder) + "/" + dummyFileName;
    binaryPath_ = std::string(folder) + "/" + "dummy.bin";
    mergedBinaryPath_ = std::string(folder) + "/" + "merged-dummy.bin";
  }

  static void testSearchItems(rime::Trie& trie) {
    testExistingWords(trie);
    testNonExistingWords(trie);
    testPrefixSearch(trie);
  }

  static void testExistingWords(rime::Trie& trie) {
    auto result1 = trie.find("accord");
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(
        result1.value(),
        "[ә'kɒ:d]; n. 一致, 调和, 协定\\n vt. 给与, 使一致\\n vi. 相符合");

    auto result2 = trie.find("accordion");
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result2.value(), "[ә'kɒ:djәn]; n. 手风琴\\n a. 可折叠的");
  }

  static void testNonExistingWords(rime::Trie& trie) {
    auto result = trie.find("nonexistent-word");
    ASSERT_FALSE(result.has_value());
  }

  static void testPrefixSearch(rime::Trie& trie) {
    auto prefixResults = trie.prefixSearch("accord");
    ASSERT_FALSE(prefixResults.empty());
    EXPECT_EQ(prefixResults.size(), 6);
  }

  std::string txtPath_;
  std::string binaryPath_;
  std::string mergedBinaryPath_;

  static constexpr size_t ENTRY_SIZE = 6;
  size_t entrySize_ = ENTRY_SIZE;

  void createDummyTextFile() const {
    std::cout << "Creating a dummy text file: " << txtPath_ << '\n';

    std::ofstream testDict(txtPath_);
    testDict << "accord	[ә'kɒ:d]; n. 一致, 调和, 协定\\n vt. 给与, "
                 "使一致\\n vi. 相符合\n";
    testDict << "accordance	[ә'kɒ:dәns]; n. 一致, 和谐\n";
    testDict
        << "according	[ә'kɒ:diŋ]; a. 相符的, 根据...而定的\\n adv. 相应地\n";
    testDict << "accordingly	[ә'kɒ:diŋli]; adv. 相应地, 因此, 于是\n";
    testDict << "accordion	[ә'kɒ:djәn]; n. 手风琴\\n a. 可折叠的\n";
    testDict << "accordionist	[ә'kɒ:djәnist]; n. 手风琴师\n";
    testDict.close();
  }

  void cleanupDummyFiles() const {
    std::cout << "Removing the dummy files\n";

    std::remove(txtPath_.c_str());
    std::remove(binaryPath_.c_str());
    std::remove((binaryPath_ + ".trie").c_str());
    std::remove(mergedBinaryPath_.c_str());
  }
};

#endif  // RIME_TRIEDATAHELPER_H_
