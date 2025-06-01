// Yes, so many constexpr... I'm getting used to it.
#ifndef TICKETSYSTEM_TS_TIME_H
#define TICKETSYSTEM_TS_TIME_H

#include "exception.h"
#include <string>

namespace ticket_system {

using minutes_count_t = int;
using days_count_t    = short;

static constexpr minutes_count_t MINUTES_PER_DAY = 1440;

class minutes;
class days;

class minutes {
private:
  minutes_count_t minute_;

public:
  using count_t = minutes_count_t;
  static constexpr minutes one_day() { return minutes(MINUTES_PER_DAY); }

  // definitions of constexpr functions must be exposed to callers.
  // normalize minute count into [0, MINUTES_PER_DAY).
  constexpr minutes& day_normalize() {
    minute_ %= MINUTES_PER_DAY;
    if(minute_ < 0)
      minute_ += MINUTES_PER_DAY;
    return *this;
  }

  constexpr minutes() : minute_(0) {}
  explicit constexpr minutes(minutes_count_t minute) : minute_(minute) {}


  [[nodiscard]] constexpr minutes_count_t count() const { return minute_; }

  constexpr minutes operator+(const minutes &other) const {
    return minutes(minute_ + other.minute_);
  }
  constexpr minutes operator-(const minutes &other) const {
    return minutes(minute_ - other.minute_);
  }
  constexpr minutes& operator+=(const minutes &other) {
    minute_ += other.minute_;
    return *this;
  }
  constexpr minutes& operator-=(const minutes &other) {
    minute_ -= other.minute_;
    return *this;
  }

  constexpr auto operator<=>(const minutes &) const = default;
};

constexpr minutes operator"" _min(unsigned long long val) { return minutes(static_cast<minutes_count_t>(val)); }

// no out of range dealed.
class days {
private:
  days_count_t day_;

public:
  using count_t = days_count_t;
  constexpr days() : day_(0) {}
  explicit constexpr days(days_count_t day) : day_(day) {}
  // days(minutes minute) : day_(minute.minute_ / 1440) {}

  [[nodiscard]] constexpr days_count_t count() const { return day_; }

  constexpr days operator+(const days &other) const {
    return days(day_ + other.day_);
  }
  constexpr days operator-(const days &other) const {
    return days(day_ - other.day_);
  }
  constexpr days& operator+=(const days &other) {
    day_ += other.day_;
    return *this;
  }
  constexpr days& operator-=(const days &other) {
    day_ -= other.day_;
    return *this;
  }

  constexpr auto operator<=>(const days &) const = default;
};

constexpr days operator"" _day(unsigned long long val) { return days(static_cast<days_count_t>(val)); }

// minutes inside be always within [0, 3600)
class TimeHM {
private:
  minutes minute_;

public:
  static std::string default_string() { return "xx:xx"; }

  constexpr TimeHM() = default;
  explicit constexpr TimeHM(minutes minute) : minute_(minute.day_normalize()) {}
  explicit constexpr TimeHM(const char *str, size_t n) {
    if(n != 5) throw insomnia::invalid_argument("invalid length for a TimeHM string.");
    minute_ = minutes(60 * (10 * (str[0] - '0') + (str[1] - '0'))
                         + (10 * (str[3] - '0') + (str[4] - '0')));
  }
  explicit constexpr TimeHM(std::string_view str)
  : minute_(60 * (10 * (str[0] - '0') + (str[1] - '0'))
               + (10 * (str[3] - '0') + (str[4] - '0'))) {}
  // TimeHM& operator=(const TimeHM &) = default; Why isn't this line needed?

  [[nodiscard]] constexpr minutes minute() const { return minute_; }
  [[nodiscard]] constexpr minutes_count_t count() const { return minute_.count(); }
  [[nodiscard]] std::string string() const;

  TimeHM operator+(minutes minutes) const {
    return TimeHM(minute_ + minutes);
  }
  TimeHM operator-(minutes minutes) const {
    return TimeHM(minute_ - minutes);
  }
  TimeHM& operator+=(minutes minutes) {
    minute_ += minutes;
    return *this;
  }
  TimeHM& operator-=(minutes minutes) {
    minute_ -= minutes;
    return *this;
  }

  constexpr auto operator<=>(const TimeHM &) const = default;
};

class DateMD {
private:
  static constexpr short MAX_DAYS = 92;
  static constexpr days DAY0701{30}, DAY0801{61}; //...

  days day_;

public:
  static std::string default_string() { return "xx-xx"; }
  constexpr DateMD() = default;
  explicit constexpr DateMD(days day) : day_(day) {}
  explicit constexpr DateMD(const char *str, size_t n) {
    if(n != 5) throw insomnia::invalid_argument("invalid length for a DateMD string.");
    day_ = days(10 * (str[3] - '0') + (str[4] - '0') - 1);
    if(str[1] == '7')      day_ += DAY0701;
    else if(str[1] == '8') day_ += DAY0801;
  }
  [[nodiscard]] constexpr days_count_t count() const { return day_.count(); }
  [[nodiscard]] constexpr days day() const { return day_; }
  [[nodiscard]] std::string string() const;

  DateMD operator+(days days) const {
    return DateMD(day_ + days);
  }
  DateMD operator-(days days) const {
    return DateMD(day_ - days);
  }
  DateMD& operator+=(days days) {
    day_ += days;
    return *this;
  }
  DateMD& operator-=(days days) {
    day_ -= days;
    return *this;
  }

  constexpr auto operator<=>(const DateMD &) const = default;
};

class DateTime {
  // don't change the order of the member variables. needed in operator<=>.
private:
  DateMD date_md_;
  TimeHM time_hm_;

public:

  static std::string default_string() {
    return DateMD::default_string() + " " + TimeHM::default_string();
  }

  constexpr DateTime() = default;
  constexpr DateTime(DateMD date_md, TimeHM time_hm) : date_md_(date_md), time_hm_(time_hm) {}

  [[nodiscard]] constexpr minutes_count_t count() const {
    return MINUTES_PER_DAY * date_md_.count() + time_hm_.count();
  }
  [[nodiscard]] constexpr TimeHM time_hm() const { return time_hm_; }
  [[nodiscard]] constexpr DateMD date_md() const { return date_md_; }
  [[nodiscard]] std::string string() const {
    return date_md_.string() + " " + time_hm_.string();
  }

  DateTime operator+(minutes minute) const;
  DateTime operator-(minutes minute) const;
  DateTime& operator+=(minutes minute);
  DateTime& operator-=(minutes minute);

  // date_md_ is in front of time_hm_.
  constexpr auto operator<=>(const DateTime &) const = default;
};

}


#endif