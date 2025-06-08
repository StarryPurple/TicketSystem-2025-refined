#include "messenger.h"

namespace insomnia {

Messenger& Messenger::operator<<(char msg) {
  msg_ += msg;
  return *this;
}

Messenger& Messenger::operator<<(const char *msg) {
  msg_ += msg;
  return *this;
}

Messenger& Messenger::operator<<(const std::string &msg) {
  msg_ += msg;
  return *this;
}

Messenger& Messenger::operator<<(std::string &&msg) {
  msg_ += std::move(msg); // NOLINT
  return *this;
}

/*
Messenger& Messenger::operator<<(const date_time_t &date_time) {
  format_to(date_time, msg_);
  return *this;
}
*/

Messenger& Messenger::append(const char *msg, size_t n) {
  msg_.append(msg, n);
  return *this;
}


}