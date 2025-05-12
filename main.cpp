#include <iostream>
#include <filesystem>

#include "vector.h"
#include "array.h"
#include "fstream.h"
#include "index_pool.h"
#include "buffer_pool.h"
#include "lru_k_replacer.h"
#include "priority_queue.h"
#include "unordered_set.h"
#include "map.h"

unsigned long hash1(const std::string &str) {
  unsigned long hash = 2166136261;
  for(const auto &c : str) {
    hash ^= c;
    hash *= 16777619;
  }
  return hash;
}

unsigned long hash2(const std::string &str) {
  unsigned long hash = 5371;
  for(const auto &c : str)
    hash = (hash << 5) + hash + c;
  return hash;
}

void print_list(insomnia::vector<int> &&list) {
  if(list.empty())
    std::cout << "null" << std::endl;
  else {
    for(int &val : list)
      std::cout << val << ' ';
    std::cout << std::endl;
  }
}

struct hash_pair {
  unsigned long h1, h2;
  hash_pair(const std::string &str) : h1(hash1(str)), h2(hash2(str)) {}
  bool operator<(const hash_pair &other) const {
    if(h1 != other.h1) return h1 < other.h1;
    return h2 < other.h2;
  }
};

/*
void BptTest() {
  using index_t = insomnia::array<char, 64>;
  using value_t = int;
  using MulBpt_t = insomnia::MultiBPlusTree<index_t, value_t>;

  auto dir = std::filesystem::current_path() / "data";
  // std::filesystem::remove_all(dir);
  std::filesystem::create_directory(dir);
  auto name_base = dir / "test";
  int k_param = 8;
  int buffer_cap = 2048;
  int thread_num = 1;
  MulBpt_t mul_bpt(name_base, k_param, buffer_cap, thread_num);

  // freopen("temp/input.txt", "r", stdin);
  // freopen("temp/output.txt", "w", stdout);

  int optcnt, value;
  index_t opt, index;
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
*/

struct T { int a; T(int _a) { a = _a; } };
struct U : T { int b; U(int _a, int _b) : T(_a) { b = _b; } };

int main() {
  // BptTest();
  auto test_dir = std::filesystem::current_path() / "data";
  std::filesystem::remove_all(test_dir);
  std::filesystem::create_directory(test_dir);
  auto test_prefix = test_dir / "test";
  insomnia::BufferPool<T, insomnia::MonoType, sizeof(U)> buf(test_prefix, 10, 2);
  auto a = buf.alloc();
  auto visitor = buf.visitor(a);
  auto p1 = visitor.as_mut<U>();
  std::cout << p1->a << ' ' << p1->b << std::endl;
  *p1 = U(1, 2);
  std::cout << p1->a << ' ' << p1->b << std::endl;
  visitor.drop();
  visitor = buf.visitor(a);
  auto p2 = visitor.as<T>();
  std::cout << p2->a << std::endl;
  return 0;
}