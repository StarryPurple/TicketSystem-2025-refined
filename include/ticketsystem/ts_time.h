#ifndef TICKETSYSTEM_TS_TIME_H
#define TICKETSYSTEM_TS_TIME_H

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

  // normalize minute into [0, 1440).
  void normalize();

public:
  static minutes one_day() { return minutes(MINUTES_PER_DAY); }

  minutes() : minute_(0) {}
  explicit minutes(minutes_count_t minute) : minute_(minute) { normalize(); }


  [[nodiscard]] minutes_count_t count() const { return minute_; }

  minutes operator+(const minutes &other) const;
  minutes operator-(const minutes &other) const;
  minutes& operator+=(const minutes &other);
  minutes& operator-=(const minutes &other);

  auto operator<=>(const minutes &) const = default;
};

constexpr minutes operator"" _min(unsigned long long val) { return minutes(static_cast<minutes_count_t>(val)); }

// no out of range dealed.
class days {
private:
  days_count_t day_;

public:
  days() : day_(0) {}
  explicit days(days_count_t day) : day_(day) {}
  // days(minutes minute) : day_(minute.minute_ / 1440) {}

  [[nodiscard]] days_count_t count() const { return day_; }

  days operator+(const days &other) const {
    return days(day_ + other.day_);
  }
  days operator-(const days &other) const {
    return days(day_ - other.day_);
  }
  days& operator+=(const days &other) {
    day_ += other.day_;
    return *this;
  }
  days& operator-=(const days &other) {
    day_ -= other.day_;
    return *this;
  }

  auto operator<=>(const days &) const = default;
};

constexpr days operator"" _day(unsigned long long val) { return days(static_cast<days_count_t>(val)); }

class TimeHM {
private:
  minutes minute_;

public:
  static std::string default_string() { return "xx:xx"; }

  TimeHM() = default;
  explicit TimeHM(minutes minute) : minute_(minute) {}
  explicit TimeHM(const std::string &str)
  : minute_(60 * (10 * (str[0] - '0') + (str[1] - '0'))
               + (10 * (str[3] - '0') + (str[4] - '0'))) {}
  TimeHM& operator=(const std::string &str) {
    minute_ = minutes(60 * (10 * (str[0] - '0') + (str[1] - '0'))
                         + (10 * (str[3] - '0') + (str[4] - '0')));
    return *this;
  }
  // TimeHM& operator=(const TimeHM &) = default;

  [[nodiscard]] minutes minute() const { return minute_; }
  [[nodiscard]] minutes_count_t count() const { return minute_.count(); }
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

  auto operator<=>(const TimeHM &) const = default;
};

class DateMD {
private:
  static constexpr days DAY0701{30}, DAY0801{61};

  days day_;

public:
  static std::string default_string() { return "xx-xx"; }
  DateMD() = default;
  explicit DateMD(days day) : day_(day) {}
  explicit DateMD(const std::string &str) {
    day_ = days(10 * (str[3] - '0') + (str[4] - '0') - 1);
    if(str[1] == '7')      day_ += DAY0701;
    else if(str[1] == '8') day_ += DAY0801;
  }
  DateMD& operator=(const std::string &str) {
    day_ = days(10 * (str[3] - '0') + (str[4] - '0') - 1);
    if(str[1] == '7')      day_ += DAY0701;
    else if(str[1] == '8') day_ += DAY0801;
    return *this;
  }
  [[nodiscard]] days_count_t count() const { return day_.count(); }
  [[nodiscard]] days day() const { return day_; }
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

  auto operator<=>(const DateMD &) const = default;
};

class DateTime {
private:
  DateMD date_md_;
  TimeHM time_hm_;

public:

  static std::string default_string() {
    return DateMD::default_string() + " " + TimeHM::default_string();
  }

  DateTime() = default;
  DateTime(DateMD date_md, TimeHM time_hm) : date_md_(date_md), time_hm_(time_hm) {}

  [[nodiscard]] minutes_count_t count() const {
    return MINUTES_PER_DAY * date_md_.count() + time_hm_.count();
  }
  [[nodiscard]] TimeHM time_hm() const { return time_hm_; }
  [[nodiscard]] DateMD date_md() const { return date_md_; }
  [[nodiscard]] std::string string() const {
    return date_md_.string() + " " + time_hm_.string();
  }

  DateTime operator+(minutes minute) const;
  DateTime operator-(minutes minute) const;
  DateTime& operator+=(minutes minute);
  DateTime& operator-=(minutes minute);
};

}


#endif