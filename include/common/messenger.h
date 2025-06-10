#ifndef INSOMNIA_MESSENGER_H
#define INSOMNIA_MESSENGER_H

#include <iostream>

#include "array.h"

namespace insomnia {

class Messenger {
public:

  Messenger() = default;
  ~Messenger() { flush(); }

  Messenger& operator<<(char msg);
  Messenger& operator<<(const char *msg);
  Messenger& operator<<(const std::string &msg);
  // Messenger& operator<<(const date_time_t &date_time);
  template<size_t N>
  Messenger& operator<<(const array<char, N> &msg);
  // Messenger& operator<<(const time_dur_t &msg);
  template <class Integer> requires std::is_integral_v<Integer>
  Messenger& operator<<(Integer val);
  Messenger& append(const char *msg, size_t n);

  void print_msg() const { std::cout << msg_; }
  void print_err() const { std::cerr << msg_; }
  [[nodiscard]]
  std::string str() const { return msg_; }
  std::string& str_ref() { return msg_; }
  void reset() { msg_.clear(); }
  void flush() { std::cout.flush(); std::cerr.flush(); }

private:
  std::string msg_;
};
}

#include "messenger.tcc"

#endif