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

inline bool is_digit(char c) { return '0' <= c && c <= '9'; }

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
  if(!is_digit(*str)) throw invalid_argument("ism::stoi");
  while(is_digit(*str)) {
    t = t * 10 + (*str - '0');
    ++str;
  }
  return sig ? t : -t;
}

template <class Integer> requires std::is_integral_v<Integer>
Integer stoi(const char *str, size_t n) {
  Integer t = 0;
  bool sig = true;
  if(*str == '-') {
    sig = false;
    ++str;
    --n;
  }
  for(size_t i = 0; i < n; ++i) {
    if(!is_digit(*str)) throw invalid_argument("ism::stoi");
    t = t * 10 + (*(str++) - '0');
  }
  return sig ? t : -t;
}

// returns whether p has advance to a delimiter
// before it hits an ending sentinel or '\0'.
// like : delimiter = "-", str = "abc--de",
// function will change *p = 'a' to *p = '-' (the first one),
// and not modify the *p = '-' (the first one).
// if returns true, it should happen that *p == delimiter.
// if returns false, *p == sentinel or '\0'.
inline bool advance_until(const char *&p, char delimiter, char sentinel = '\0') {
  while(*p != delimiter && *p != sentinel && *p != '\0') ++p;
  return *p == delimiter;
}

// returns whether p has skipped (continuous) delimiter(s)
// before it hits an ending sentinel or '\0'.
// like : delimiter = "-", str = "abc--de",
// function will change *p = 'a' to *p = 'd',
// change *p = '-' (the first one) to *p = 'd'.
// if returns true, *p != delimiter and *(p - 1) == delimiter.
// if returns false, *p == sentinel or '\0'.
inline bool advance_past(const char *&p, char delimiter, char sentinel = '\0') {
  while(*p != delimiter && *p != sentinel && *p != '\0') ++p;
  while(*p == delimiter) ++p;
  return *p != sentinel && *p != '\0';
}

}

#include "algorithm.tcc"

#endif