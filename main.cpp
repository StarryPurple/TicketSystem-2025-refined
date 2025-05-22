#include <cassert>
#include <iostream>
#include <filesystem>
#include <mutex>

#include "vector.h"
#include "array.h"
#include "multi_bplustree.h"

u_long hash1(const std::string &str) {
  u_long hash = 2166136261;
  for(const auto &c : str) {
    hash ^= c;
    hash *= 16777619;
  }
  return hash;
}

u_long hash2(const std::string &str) {
  u_long hash = 5371;
  for(const auto &c : str)
    hash = (hash << 5) + hash + c;
  return hash;
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
  using index_t = insomnia::array<char, 64>;
  using value_t = int;
  using MulBpt_t = insomnia::MultiBpt<index_t, value_t>;

  auto dir = std::filesystem::current_path() / "data";
  // std::filesystem::remove_all(dir);
  std::filesystem::create_directory(dir);
  auto name_base = dir / "test";
  int replacer_k_arg = 4;
  int buffer_capacity = 4096;
  MulBpt_t mul_bpt(name_base, buffer_capacity, replacer_k_arg);

  // freopen("temp/input.txt", "r", stdin);
  // freopen("temp/output.txt", "w", stdout);

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

  // system("diff -bB temp/output.txt temp/answer.txt");
}

void SaferBptTest() {
  try {
    BptTest();
  } catch(insomnia::invalid_page &) {
    std::cout << "err";
  }
}

int main() {
  BptTest();
  return 0;
}