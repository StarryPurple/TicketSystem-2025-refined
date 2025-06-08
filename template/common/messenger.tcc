#ifndef INSOMNIA_MESSENGER_TCC
#define INSOMNIA_MESSENGER_TCC

#include "messenger.h"

namespace insomnia {

template <size_t N>
Messenger& Messenger::operator<<(const array<char, N> &msg) {
  msg_.append(msg.c_str(), msg.length());
  return *this;
}

template <class Integer> requires std::is_integral_v<Integer>
Messenger& Messenger::operator<<(Integer val) {
  // C++17 std::to_chars are said to be better.
  // remove "static" if multi threads are enabled.
  static char buf[64];
  char *p = buf + sizeof(buf);
  // *(--p) = '\0'; not needed.
  bool is_neg = false;
  if(val == 0)
    *(--p) = '0';
  else {
    if(val < 0) {
      is_neg = true;
      val = -val;
    }
    while(val > 0) {
      *(--p) = (val % 10) + '0';
      val /= 10;
    }
  }
  if(is_neg)
    *(--p) = '-';
  msg_.append(p, buf + sizeof(buf) - p);
  return *this;
}

}


#endif
