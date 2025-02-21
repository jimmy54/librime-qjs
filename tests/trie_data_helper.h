#ifndef RIME_TRIEDATAHELPER_H_
#define RIME_TRIEDATAHELPER_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <gtest/gtest.h>
#include "trie.h"

class TrieDataHelper {

public:
    TrieDataHelper(const char* folder, const char* dummyFileName) {
        if (!dummyFileName) return;

        txtPath_ = std::string(folder) + "/" + dummyFileName;
        binaryPath_ = std::string(folder) + "/" + "dummy.bin";
        mergedBinaryPath_ = std::string(folder) + "/" + "merged-dummy.bin";
    }


    void TestSearchItems(rime::Trie& trie) {
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

    std::string txtPath_;
    std::string binaryPath_;
    std::string mergedBinaryPath_;
    size_t entrySize_ = 6;

    void createDummyTextFile() {
        std::cout << "Creating a dummy text file: " << txtPath_ << std::endl;

        std::ofstream test_dict(txtPath_);
        test_dict << "accord	[ә'kɒ:d]; n. 一致, 调和, 协定\\n vt. 给与, 使一致\\n vi. 相符合\n";
        test_dict << "accordance	[ә'kɒ:dәns]; n. 一致, 和谐\n";
        test_dict << "according	[ә'kɒ:diŋ]; a. 相符的, 根据...而定的\\n adv. 相应地\n";
        test_dict << "accordingly	[ә'kɒ:diŋli]; adv. 相应地, 因此, 于是\n";
        test_dict << "accordion	[ә'kɒ:djәn]; n. 手风琴\\n a. 可折叠的\n";
        test_dict << "accordionist	[ә'kɒ:djәnist]; n. 手风琴师\n";
        test_dict.close();
    }

    void cleanupDummyFiles() {
        std::cout << "Removing the dummy files" << std::endl;

        std::remove(txtPath_.c_str());
        std::remove(binaryPath_.c_str());
        std::remove((binaryPath_ + ".trie").c_str());
        std::remove(mergedBinaryPath_.c_str());
    }
};

#endif // RIME_TRIEDATAHELPER_H_
