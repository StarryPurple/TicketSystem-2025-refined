#ifndef INSOMNIA_ALGORITHM_TCC
#define INSOMNIA_ALGORITHM_TCC

#include "algorithm.h"

namespace insomnia {

template <class RAIt, class Compare>
void insertion_sort(RAIt begin, RAIt end, Compare comp) {
  if(begin == end) return;
  for(auto i = begin + 1; i != end; ++i) {
    auto val = std::move(*i);
    auto j = i;
    while(j > begin && comp(val, *(j - 1))) {
      *j = std::move(*(j - 1));
      --j;
    }
    *j = std::move(val);
  }
}

template <class T, class Compare>
T median_of_three(const T &a, const T &b, const T &c, Compare comp) {
  return comp(a, b) ?
    (comp(b, c) ? b : (comp(a, c) ? c : a)) :
    (comp(a, c) ? a : (comp(b, c) ? c : b));
}

template <class RAIt, class Compare>
std::pair<RAIt, RAIt> partition(RAIt begin, RAIt end, Compare comp) {
  auto mid = begin + (end - begin) / 2;
  auto &&pivot = median_of_three(*begin, *mid, *(end - 1), comp);
  auto l = begin, r = end, p = begin;
  while(p < r) {
    if(comp(*p, pivot))
      std::iter_swap(l++, p++);
    else if(comp(pivot, *p))
      std::iter_swap(p, --r);
    else
      ++p;
  }
  return {l, r};
}

template <class RAIt, class Compare>
void sort(RAIt begin, RAIt end, Compare comp) {
  constexpr size_t INSERTION_THRESHOLD = 8;
  while(end - begin > INSERTION_THRESHOLD) {
    auto [eq_begin, eq_end] = partition(begin, end, comp);
    if(eq_begin - begin < end - eq_end) {
      sort(begin, eq_begin, comp);
      begin = eq_end;
    } else {
      sort(eq_end, end, comp);
      end = eq_begin;
    }
  }
  insertion_sort(begin, end, comp);
}

}

#endif