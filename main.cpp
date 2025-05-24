#include <iostream>
#include <filesystem>
#include <mutex>

#include "vector.h"
#include "array.h"
#include "multi_bplustree.h"

uint64_t hash1(const std::string &str) {
  uint64_t hash = 2166136261;
  for(const auto &c : str) {
    hash ^= c;
    hash *= 16777619;
  }
  return hash;
}

uint64_t hash2(const std::string &str) {
  uint64_t hash = 5371;
  for(const auto &c : str)
    hash = (hash << 5) + hash + c;
  return hash;
}

uint64_t hash3(const std::string &str) {
  uint64_t hash = 0xCBF29CE484222325ULL;
  for (const auto &c : str) {
    hash ^= static_cast<uint64_t>(c);
    hash *= 0x100000001B3ULL;
  }
  return hash ^ (hash >> 32);
}

void print_list(insomnia::vector<int> &&list) {
  if(list.empty())
    std::cout << "null";
  else {
    for(auto &val : list)
      std::cout << val << ' ';
  }
  std::cout << '\n';
}

void BptTest() {
  using index_t = insomnia::array<char, 127>;
  using value_t = int;
  using MulBpt_t = insomnia::MultiBpt<index_t, value_t>;

  auto dir = std::filesystem::current_path() / "data";
  std::filesystem::create_directory(dir);
  auto name_base = dir / "test";
  int replacer_k_arg = 4;
  int buffer_capacity = 1536;
  MulBpt_t mul_bpt(name_base, buffer_capacity, replacer_k_arg);

  int optcnt;
  int value;
  std::string opt, index;
  std::cin >> optcnt;
  for(int i = 1; i <= optcnt; ++i) {
    std::cin >> opt;
    if(opt[0] == 'i') {
      std::cin >> index >> value;
      mul_bpt.insert(index, value);
    } else if(opt[0] == 'f') {
      std::cin >> index;
      print_list(mul_bpt.search(index));
    } else if(opt[0] == 'd') {
      std::cin >> index >> value;
      mul_bpt.remove(index, value);
    }
  }
}

void SaferBptTest() {
  try {
    BptTest();
  } catch(...) {
    std::cout << "err";
  }
}

void MultitaskBptTest() {
  using index_t = insomnia::array<char, 65>;
  using value_t = int;
  using MulBpt_t = insomnia::MultiBpt<index_t, value_t>;

  auto data_dir = std::filesystem::current_path() / "data";
  std::filesystem::remove_all(data_dir);
  std::filesystem::create_directory(data_dir);
  auto name_base = data_dir / "test";
  int replacer_k_arg = 4;
  int buffer_capacity = 512;

  auto subtest_dir = std::filesystem::current_path() / "subtest";
  std::vector<std::filesystem::path> input_files;
  for(auto &entry : std::filesystem::directory_iterator(subtest_dir))
    if(entry.path().extension() == ".in") {
      input_files.push_back(entry.path());
    }

  std::sort(input_files.begin(), input_files.end());
  for(auto &input_file : input_files) {
    auto output_file = input_file;
    output_file.replace_extension(".out");

    std::ifstream fin(input_file);
    std::ofstream fout(output_file);
    MulBpt_t mul_bpt(name_base, buffer_capacity, replacer_k_arg);

    int optcnt;
    int value;
    std::string opt, index;
    fin >> optcnt;
    for(int i = 1; i <= optcnt; ++i) {
      fin >> opt;
      if(opt[0] == 'i') {
        fin >> index >> value;
        mul_bpt.insert(index, value);
      } else if(opt[0] == 'f') {
        fin >> index;
        auto lst = mul_bpt.search(index);
        if(lst.empty()) fout << "null";
        else {
          for(auto &val : lst)
            fout << val << ' ';
        }
        fout << std::endl;
      } else if(opt[0] == 'd') {
        fin >> index >> value;
        mul_bpt.remove(index, value);
      }
    }
    fin.close();
    fout.close();
  }
}

int main() {
  BptTest();
  return 0;
}