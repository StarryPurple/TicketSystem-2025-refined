#include <cassert>
#include <iostream>
#include <filesystem>

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
    std::cout << "null" << std::endl;
  else {
    for(int &val : list)
      std::cout << val << ' ';
    std::cout << std::endl;
  }
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
  int buffer_capacity = 512;
  MulBpt_t mul_bpt(name_base, buffer_capacity, replacer_k_arg);

  // freopen("temp/input.txt", "r", stdin);
  // freopen("temp/output.txt", "w", stdout);

  int optcnt, value;
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
  } catch(insomnia::debug_exception &) {
    std::cout << "err";
  }
}

#ifndef ASSERT_EQ
#define ASSERT_EQ(expr1, expr2) assert(expr1 == expr2)

void ReplacerTest() {
  insomnia::LruKReplacer lru_replacer(7, 2);

  // Add six frames to the replacer. We now have frames [1, 2, 3, 4, 5]. We set frame 6 as non-evictable.
  lru_replacer.access(1);
  lru_replacer.access(2);
  lru_replacer.access(3);
  lru_replacer.access(4);
  lru_replacer.access(5);
  lru_replacer.access(6);
  lru_replacer.unpin(1);
  lru_replacer.unpin(2);
  lru_replacer.unpin(3);
  lru_replacer.unpin(4);
  lru_replacer.unpin(5);
  lru_replacer.pin(6);

  // The size of the replacer is the number of frames that can be evicted, _not_ the total number of frames entered.
  ASSERT_EQ(5, lru_replacer.free_cnt());

  // Record an access for frame 1. Now frame 1 has two accesses total.
  lru_replacer.access(1);
  // All other frames now share the maximum backward k-distance. Since we use timestamps to break ties, where the first
  // to be evicted is the frame with the oldest timestamp, the order of eviction should be [2, 3, 4, 5, 1].

  // Evict three pages from the replacer.
  // To break ties, we use LRU with respect to the oldest timestamp, or the least recently used frame.
  ASSERT_EQ(2, lru_replacer.evict());
  ASSERT_EQ(3, lru_replacer.evict());
  ASSERT_EQ(4, lru_replacer.evict());
  ASSERT_EQ(2, lru_replacer.free_cnt());
  // Now the replacer has the frames [5, 1].

  // Insert new frames [3, 4], and update the access history for 5. Now, the ordering is [3, 1, 5, 4].
  lru_replacer.access(3);
  lru_replacer.access(4);
  lru_replacer.access(5);
  lru_replacer.access(4);
  lru_replacer.unpin(3);
  lru_replacer.unpin(4);
  ASSERT_EQ(4, lru_replacer.free_cnt());

  // Look for a frame to evict. We expect frame 3 to be evicted next.
  ASSERT_EQ(3, lru_replacer.evict());
  ASSERT_EQ(3, lru_replacer.free_cnt());

  // Set 6 to be evictable. 6 Should be evicted next since it has the maximum backward k-distance.
  lru_replacer.unpin(6);
  ASSERT_EQ(4, lru_replacer.free_cnt());
  ASSERT_EQ(6, lru_replacer.evict());
  ASSERT_EQ(3, lru_replacer.free_cnt());

  // Mark frame 1 as non-evictable. We now have [5, 4].
  lru_replacer.pin(1);

  // We expect frame 5 to be evicted next.
  ASSERT_EQ(2, lru_replacer.free_cnt());
  ASSERT_EQ(5, lru_replacer.evict());
  ASSERT_EQ(1, lru_replacer.free_cnt());

  // Update the access history for frame 1 and make it evictable. Now we have [4, 1].
  lru_replacer.access(1);
  lru_replacer.access(1);
  lru_replacer.unpin(1);
  ASSERT_EQ(2, lru_replacer.free_cnt());

  // Evict the last two frames.
  ASSERT_EQ(4, lru_replacer.evict());
  ASSERT_EQ(1, lru_replacer.free_cnt());
  ASSERT_EQ(1, lru_replacer.evict());
  ASSERT_EQ(0, lru_replacer.free_cnt());

  // Insert frame 1 again and mark it as non-evictable.
  lru_replacer.access(1);
  lru_replacer.pin(1);
  ASSERT_EQ(0, lru_replacer.free_cnt());

  // A failed eviction should not change the size of the replacer.
  auto frame = lru_replacer.evict();
  ASSERT_EQ(7, frame);

  // Mark frame 1 as evictable again and evict it.
  lru_replacer.unpin(1);
  ASSERT_EQ(1, lru_replacer.free_cnt());
  ASSERT_EQ(1, lru_replacer.evict());
  ASSERT_EQ(0, lru_replacer.free_cnt());

  // There is nothing left in the replacer, so make sure this doesn't do something strange.
  frame = lru_replacer.evict();
  ASSERT_EQ(7, frame);
  ASSERT_EQ(0, lru_replacer.free_cnt());

  // Make sure that setting a non-existent frame as evictable or non-evictable doesn't do something strange.
  lru_replacer.pin(6);
  lru_replacer.unpin(6);
}

#endif

int main() {
  BptTest();
  return 0;
}