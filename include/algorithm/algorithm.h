#ifndef INSOMNIA_ALGORITHM_H
#define INSOMNIA_ALGORITHM_H

namespace insomnia {

template <class RAIt, class Compare>
void sort(RAIt begin, RAIt end, Compare comp);

template <class RAIt>
void sort(RAIt begin, RAIt end) {
  using compare_t = std::less<typename std::iterator_traits<RAIt>::value_type>;
  sort(begin, end, compare_t());
}

bool is_digit(char c) { return '0' <= c && c <= '9'; }

template <class Integer> requires std::is_integral_v<Integer>
Integer stoi(const std::string &str) {
  Integer t = 0;
  bool sig = true;
  auto cur = str.begin();
  if(*cur == '-') {
    sig = false;
    ++cur;
  }
  if(!is_digit(*cur)) throw invalid_argument("insomnia stoi");
  while(cur != str.end()) {
    t = t * 10 + (*cur - '0');
    ++cur;
  }
  return sig ? t : -t;
}

template <class Integer> requires std::is_integral_v<Integer>
Integer stoi(const char *str) {
  Integer t = 0;
  bool sig = true;
  if(*str == '-') {
    sig = false;
    ++str;
  }
  if(!is_digit(*str)) throw invalid_argument("insomnia stoi");
  while(is_digit(*str)) {
    t = t * 10 + (*str - '0');
    ++str;
  }
  return sig ? t : -t;
}

}

#include "algorithm.tcc"

#endif