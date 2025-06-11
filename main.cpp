#include <iostream>
#include <filesystem>

#include "multi_bplustree.h"
#include "ticketsystem.h"

void MultiBptTest();
void TicketSystemTest();

int main() {
  TicketSystemTest();
  return 0;
}

/*********** implementations ************/

uint64_t hash1(const std::string &str) {
  uint64_t hash = 2166136261;
  for(const auto &c : str) {
    hash ^= c;
    hash *= 16777619;
  }
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

namespace ism = insomnia;
namespace fs = std::filesystem;

void MultiBptTest() {
  using str_t = ism::array<char, 64>;
  using index_t = uint64_t;
  using value_t = int;
  using MulBpt_t = ism::MultiBplustree<index_t, value_t>;

  auto dir = fs::current_path() / "data";
  fs::create_directory(dir);
  auto name_base = dir / "test";
  int replacer_k_arg = 4;
  int buffer_capacity = 1536;
  MulBpt_t mul_bpt(name_base, buffer_capacity, replacer_k_arg);

  int optcnt;
  int value;
  std::string opt;
  std::string index;
  std::cin >> optcnt;
  for(int i = 1; i <= optcnt; ++i) {
    std::cin >> opt;
    if(opt[0] == 'i') {
      std::cin >> index >> value;
      mul_bpt.insert(hash1(index), value);
    } else if(opt[0] == 'f') {
      std::cin >> index;
      print_list(mul_bpt.search(hash1(index)));
    } else if(opt[0] == 'd') {
      std::cin >> index >> value;
      mul_bpt.remove(hash1(index), value);
    }
  }
}

namespace ts = ticket_system;

void TicketSystemTest() {
  auto data_dir = fs::current_path() / "ts_data";
  // fs::remove_all(data_dir);
  fs::create_directory(data_dir);
  auto name_base = data_dir / "ts";
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  ts::TicketSystem ticket_system(name_base);
  ticket_system.work_loop();
}