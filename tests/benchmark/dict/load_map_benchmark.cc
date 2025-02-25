#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <cstdio>
#include <string_view>
#include <yas/serialize.hpp>
#include <yas/std_types.hpp>
#include <yas/binary_oarchive.hpp>
#include <yas/binary_iarchive.hpp>

#include "benchmark_helper.h"

#include "trie.h"
#include "map.h"

// #define RUN_BENCHMARK_WITH_REAL_DATA

#ifndef RUN_BENCHMARK_WITH_REAL_DATA
std::string txtPath_ = "./tests/benchmark/dict/dummy_dict.txt";
#else
std::string txtPath_ = "/Users/hj/Library/Rime/lua/data/cedict_fixed.u8";
#endif

std::string folder_ = "./tests/benchmark/dict/";

class GlobalEnvironment : public testing::Environment {
public:
  void SetUp() override {
    std::cout << "Creating a dummy text file: " << txtPath_ << std::endl;

    std::ofstream test_dict(txtPath_);
    test_dict << "点头\t[diǎn tóu]to nod\n";
    test_dict << "点头之交\t[diǎn tóu zhī jiāo]nodding acquaintance\n";
    test_dict << "点头咂嘴\t[diǎn tóu zā zuǐ]to approve by nodding one's head and smacking one's lips (idiom)\n";
    test_dict << "点头哈腰\t[diǎn tóu hā yāo]to nod one's head and bow (idiom); bowing and scraping/unctuous fawning\n";
    test_dict << "点头招呼\t[diǎn tóu zhāo hū]beckon\n";
    test_dict << "点题\t[diǎn tí]to bring out the main theme/to make the point/to bring out the substance concisely\n";
    test_dict << "点餐\t[diǎn cān](at a restaurant) to order a meal/(of a waiter) to take an order\n";
    test_dict << "点鬼火\t[diǎn guǐ huǒ]to stir up trouble in secret/to instigate\n";
    test_dict << "点点\t[diǎn diǎn]Diandian (Chinese microblogging and social networking website)||[diǎn diǎn]point/speck\n";
    test_dict << "点点滴滴\t[diǎn diǎn dī dī]bit by bit/dribs and drabs/the little details/every aspect\n";
    test_dict.close();
  }

  void TearDown() override {
    std::remove(txtPath_.c_str());
  }
};

void checkMapData(const std::unordered_map<std::string, std::string>& map) {
  auto val = map.at("点头之交");
  ASSERT_EQ(val, "[diǎn tóu zhī jiāo]nodding acquaintance");
  val = map.at("点头");
  ASSERT_EQ(val, "[diǎn tóu]to nod");
  val = map.at("点点滴滴");
  ASSERT_EQ(val, "[diǎn diǎn dī dī]bit by bit/dribs and drabs/the little details/every aspect");
  val = map.at("点点");
  ASSERT_EQ(val, "[diǎn diǎn]Diandian (Chinese microblogging and social networking website)||[diǎn diǎn]point/speck");

  auto notFound = map.find("abc");
  ASSERT_EQ(notFound, map.end());
  notFound = map.find("");
  ASSERT_EQ(notFound, map.end());
}
void checkTrieData(rime::Trie& trie) {
  auto val = trie.find("点头之交");
  ASSERT_TRUE(val.has_value());
  ASSERT_EQ(val.value(), "[diǎn tóu zhī jiāo]nodding acquaintance");
  val = trie.find("点头");
  ASSERT_TRUE(val.has_value());
  ASSERT_EQ(val.value(), "[diǎn tóu]to nod");
  val = trie.find("点点滴滴");
  ASSERT_TRUE(val.has_value());
  ASSERT_EQ(val.value(), "[diǎn diǎn dī dī]bit by bit/dribs and drabs/the little details/every aspect");
  val = trie.find("点点");
  ASSERT_TRUE(val.has_value());
  ASSERT_EQ(val.value(), "[diǎn diǎn]Diandian (Chinese microblogging and social networking website)||[diǎn diǎn]point/speck");

  auto notFound = trie.find("abc");
  ASSERT_FALSE(notFound.has_value());
  auto emptyKey = trie.find("");
  ASSERT_TRUE(emptyKey.has_value());
}

std::unordered_map<std::string, std::string> loadTextToMap(const std::string& filePath, size_t dictSize) {
  std::unordered_map<std::string, std::string> map;
  map.reserve(dictSize);

  std::ifstream infile(filePath);
  std::string line;
  while (std::getline(infile, line)) {
    if (!line.empty() && line[0] == '#') continue;

    size_t tab_pos = line.find('\t');
    if (tab_pos != std::string::npos) {
      auto lineView = std::string_view(line);
      std::string_view key_view = lineView.substr(0, tab_pos);
      std::string_view value_view = lineView.substr(tab_pos + 1);
      if (value_view.back() == '\r') {
        value_view.remove_suffix(1);
      }

      std::string key{ key_view };
      std::string value{ value_view };
      map.insert_or_assign(std::move(key), std::move(value));
    }
  }
  return map;
}

void saveMapWithYas(const std::string& filename, const std::unordered_map<std::string, std::string>& map) {
  yas::mem_ostream os;
  yas::binary_oarchive<yas::mem_ostream> oa(os);
  oa(map);

  std::ofstream fout(filename, std::ios::binary);
  const auto buf = os.get_shared_buffer();
  fout.write(reinterpret_cast<const char*>(buf.data.get()), buf.size);
}

std::unordered_map<std::string, std::string> loadMapWithYas(const std::string& filename) {
  std::ifstream fin(filename, std::ios::binary | std::ios::ate);
  std::streamsize size = fin.tellg();
  fin.seekg(0, std::ios::beg);

  std::vector<char> buffer(size);
  fin.read(buffer.data(), size);

  yas::mem_istream is(buffer.data(), buffer.size());
  yas::binary_iarchive<yas::mem_istream> ia(is);

  std::unordered_map<std::string, std::string> map;
  ia(map);
  return map;
}

size_t dictSize_ = 119000;

TEST(LoadMapDictBenchmark, LoadToTrie) {
  { // warm up
    rime::Trie trie;
    trie.loadTextFile(txtPath_, dictSize_);

    auto map = loadTextToMap(txtPath_, dictSize_);
  }

  rime::Trie trie;
  PRINT_DURATION(MAGENTA, "Plain text to Trie: \t",
    trie.loadTextFile(txtPath_, dictSize_)
  );
  checkTrieData(trie);

  auto trieFile = folder_ + "dict_map.trie";
  RESAVE_FILE(trieFile,
    PRINT_DURATION(YELLOW, "Trie serialization: \t",
      trie.saveToBinaryFile(trieFile)
    )
  );

  rime::Trie trie2;
  PRINT_DURATION(MAGENTA, "Trie deserialization: \t",
    trie2.loadBinaryFileMmap(trieFile)
  );
  checkTrieData(trie2);
  std::remove(trieFile.c_str());
}

TEST(LoadMapDictBenchmark, LoadTextFileAndLookup) {
  std::unordered_map<std::string, std::string> map;
  PRINT_DURATION(MAGENTA, "Plain text to Map: \t",
    map = loadTextToMap(txtPath_, dictSize_)
  );
  checkMapData(map);

  MmapStringMap mmap;
  auto mmapFile = folder_ + "dict_map.mmap";
  RESAVE_FILE(mmapFile,
    PRINT_DURATION(YELLOW, "Mmap serialization: \t",
      mmap.save(mmapFile, map)
    )
  );

  auto yasFile = folder_ + "dict_map.yas";
  RESAVE_FILE(yasFile,
    PRINT_DURATION(YELLOW, "YAS serialization: \t",
      saveMapWithYas(yasFile, map)
    )
  );
}

TEST(LoadMapDictBenchmark, LoadMmap) {
  MmapStringMap mmap;
  std::unordered_map<std::string, std::string> map;

  auto mmapFile = folder_ + "dict_map.mmap";
  PRINT_DURATION(MAGENTA, "Mmap deserialization: \t",
    map = mmap.load(mmapFile)
  );
  checkMapData(map);
  std::remove(mmapFile.c_str());
}

TEST(LoadMapDictBenchmark, LoadYas) {
  auto yasFile = folder_ + "dict_map" + ".yas";
  std::unordered_map<std::string, std::string> map;
  PRINT_DURATION(MAGENTA, "Mmap deserialization: \t",
    map = loadMapWithYas(yasFile)
  );
  checkMapData(map);
  std::remove(yasFile.c_str());
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);

#ifndef RUN_BENCHMARK_WITH_REAL_DATA
  testing::AddGlobalTestEnvironment(new GlobalEnvironment);
#endif

  return RUN_ALL_TESTS();
}

// Benchmark of loading the cn2en dictionary
// +=============================================================================================================+
// | Option               | Load from Text | Save to Binary | Load from Binary |  Hardware                       |
// +----------------------+----------------+----------------+------------------+---------------------------------+
// | Trie (with mmap)     |  210 ms        |  50 ms         |  15 ms           | MBP 2015, 2.4 GHz Intel Core i7 |
// | unordered_map + mmap |  110 ms        |  45 ms         |  50 ms           | MBP 2015, 2.4 GHz Intel Core i7 |
// | unordered_map + YAS  |  110 ms        |  53 ms         |  70 ms           | MBP 2015, 2.4 GHz Intel Core i7 |
// +=============================================================================================================+
