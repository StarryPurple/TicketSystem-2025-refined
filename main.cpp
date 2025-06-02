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
  using index_t = str_t;
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

ism::vector<ism::pair<std::string, ism::vector<std::string>>> tests = {
  {std::string("basic_1"), ism::vector<std::string>{"1"}},
  {std::string("basic_2"), ism::vector<std::string>{"2"}},
  {std::string("basic_3"), ism::vector<std::string>{"3", "4", "5", "6", "7"}},
  {std::string("basic_4"), ism::vector<std::string>{"8", "9", "10", "11", "12"}},
  {std::string("basic_5"), ism::vector<std::string>{"13", "14", "15", "16", "17", "18", "19", "20", "21", "22"}},
  {std::string("basic_6"), ism::vector<std::string>{"23", "24", "25", "26", "27", "28", "29", "30", "31", "32"}},
  {std::string("basic_extra"), ism::vector<std::string>{"33", "34", "35", "36", "37", "38", "39", "40", "41", "42"}},
  {std::string("pressure_1_easy"), ism::vector<std::string>{"43", "44", "45", "46", "47", "48", "49", "50", "51", "52"}},
  {std::string("pressure_2_easy"), ism::vector<std::string>{"53", "54", "55", "56", "57", "58", "59", "60", "61", "62"}},
  {std::string("pressure_3_easy"), ism::vector<std::string>{"63", "64", "65", "66", "67", "68", "69", "70", "71", "72"}},
  {std::string("pressure_1_hard"), ism::vector<std::string>{"73", "74", "75", "76", "77", "78", "79", "80", "81", "82"}},
  {std::string("pressure_2_hard"), ism::vector<std::string>{"83", "84", "85", "86", "87", "88", "89", "90", "91", "92"}},
  {std::string("pressure_3_hard"), ism::vector<std::string>{"93", "94", "95", "96", "97", "98", "99", "100", "101", "102"}}
};

void LocalTicketSystemTest() {
  auto data_dir = fs::current_path() / "ts_localtest";
  auto name_base = data_dir / "ts";
  auto localtest_dir = fs::current_path().parent_path() / "localtest";

  std::string result_file = localtest_dir / "0-result.txt";
  std::string output_file = localtest_dir / "0-output.txt";
  std::string differ_file = localtest_dir / "0-differ.txt";
  system(("echo \"Test starts.\" >" + result_file).c_str());

  for(const auto &[name, pack] : tests) {
    fs::remove_all(data_dir);
    fs::create_directory(data_dir);
    system(("echo \"-------Starting test " + name + "-------\" >> " + result_file).c_str());

    for(const auto &test_no : pack) {
      std::string input_file = localtest_dir / (test_no + ".in");
      std::string answer_file = localtest_dir / (test_no + ".out");

      std::ifstream input(input_file);
      std::ofstream output(output_file);

      auto *old_cin_buf = std::cin.rdbuf();
      auto *old_cout_buf = std::cout.rdbuf();

      std::cin.rdbuf(input.rdbuf());
      std::cout.rdbuf(output.rdbuf());

      ts::TicketSystem ticket_system(name_base);
      ticket_system.work_loop();

      std::cin.rdbuf(old_cin_buf);
      std::cout.rdbuf(old_cout_buf);

      std::string diff_cmd = "diff -bB " + output_file + " " + answer_file + " > " + differ_file;
      system(diff_cmd.c_str());
      if(fs::file_size(differ_file) == 0)
        system(("echo \"Test point " + test_no + " passed.\" >> " + result_file).c_str());
      else {
        std::cout << "Test point " + test_no + " failed. \nCheck differ file for more details." << std::endl;
        system(("echo \"Test point " + test_no + " failed.\" >> " + result_file).c_str());
        system(("echo \"Check differ file for more details.\" >> " + result_file).c_str());
        break;
      }
    }
    system(("echo \"\" >> " + result_file).c_str());
  }
}

int main() {
  MultiBptTest();
  return 0;
}