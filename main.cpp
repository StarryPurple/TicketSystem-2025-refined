#include <iostream>
#include <filesystem>

#include "vector.h"
#include "array.h"
#include "algorithm.h"
#include "multi_bplustree.h"
#include "bplustree.h"

#include "ts_types.h"
#include "ts_time.h"
#include "ticketsystem.h"

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

void SaferMultiBptTest() {
  try {
    MultiBptTest();
  } catch(...) {
    std::cout << "err";
  }
}
void MultitaskMultiBptTest() {
  using index_t = ism::array<char, 64>;
  using value_t = int;
  using MulBpt_t = ism::MultiBplustree<index_t, value_t>;

  auto data_dir = fs::current_path() / "data";
  fs::remove_all(data_dir);
  fs::create_directory(data_dir);
  auto name_base = data_dir / "test";
  int replacer_k_arg = 4;
  int buffer_capacity = 512;

  auto subtest_dir = fs::current_path() / "subtest";
  ism::vector<fs::path> input_files;
  for(auto &entry : fs::directory_iterator(subtest_dir))
    if(entry.path().extension() == ".in") {
      input_files.push_back(entry.path());
    }

  insomnia::sort(input_files.begin(), input_files.end());
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

void MultitaskBptTest() {
  using index_t = ism::array<char, 64>;
  using value_t = int;
  using Bpt_t = ism::Bplustree<index_t, value_t>;

  auto data_dir = fs::current_path() / "data";
  fs::remove_all(data_dir);
  fs::create_directory(data_dir);
  auto name_base = data_dir / "test";
  int replacer_k_arg = 4;
  int buffer_capacity = 512;

  auto subtest_dir = fs::current_path() / "subtest";
  ism::vector<fs::path> input_files;
  for(auto &entry : fs::directory_iterator(subtest_dir))
    if(entry.path().extension() == ".in") {
      input_files.push_back(entry.path());
    }

  ism::sort(input_files.begin(), input_files.end());
  for(auto &input_file : input_files) {
    auto output_file = input_file;
    output_file.replace_extension(".out");

    std::ifstream fin(input_file);
    std::ofstream fout(output_file);
    Bpt_t mul_bpt(name_base, buffer_capacity, replacer_k_arg);

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
        auto res = mul_bpt.search(index);
        if(!res.has_value()) fout << "null";
        else fout << *res;
        fout << std::endl;
      } else if(opt[0] == 'd') {
        fin >> index;
        mul_bpt.remove(index);
      }
    }
    fin.close();
    fout.close();
  }
}

namespace ts = ticket_system;

void TicketSystemTest() {
  auto data_dir = fs::current_path() / "ts_data";
  // fs::remove_all(data_dir);
  fs::create_directory(data_dir);
  auto name_base = data_dir / "ts";
  ts::TicketSystem ticket_system(name_base);
  ticket_system.work_loop();
}

void LocalTicketSystemTest() {
  auto data_dir = fs::current_path() / "ts_test";
  fs::remove_all(data_dir);
  fs::create_directory(data_dir);
  auto name_base = data_dir / "ts";
  ts::TicketSystem ticket_system(name_base);
  auto localtest_dir = fs::current_path().parent_path() / "localtest";

  std::string input_file = localtest_dir / "2.in";
  std::string answer_file = localtest_dir / "2.out";
  std::string output_file = localtest_dir / "0-output.txt";

  freopen(input_file.c_str(), "r", stdin);
  freopen(output_file.c_str(), "w", stdout);
  ticket_system.work_loop();
  fclose(stdin);
  fclose(stdout);

  std::string diff_file = localtest_dir / "0-diff.txt";

  std::string diff_cmd = "diff -bB " + output_file + " " + answer_file + " > " + diff_file;
  system(diff_cmd.c_str());
  fclose(stdout);
}

int main() {
  LocalTicketSystemTest();
  return 0;
}