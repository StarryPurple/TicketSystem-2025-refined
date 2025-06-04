#include "ts_time.h"
#include "algorithm.h"

namespace ticket_system {

std::string TimeHM::string() const {
  std::string res;
  auto hour = count() / 60, minute = count() - 60 * hour;
  res += (hour / 10) + '0';
  res += (hour % 10) + '0';
  res += ':';
  res += (minute / 10) + '0';
  res += (minute % 10) + '0';
  return res;
}

void format_to(const TimeHM &time_hm, std::string &out) {
  auto hour = time_hm.count() / 60, minute = time_hm.count() - 60 * hour;
  out += (hour / 10) + '0';
  out += (hour % 10) + '0';
  out += ':';
  out += (minute / 10) + '0';
  out += (minute % 10) + '0';
}

std::string DateMD::string() const {
  std::string res;
  days day = day_;
  if(day >= DAY0901) {
    res += "09-"; day -= DAY0901;
  } else if(day >= DAY0801) {
    res += "08-"; day -= DAY0801;
  } else if(day >= DAY0701) {
    res += "07-"; day -= DAY0701;
  } else {
    res += "06-";
  }
  day += 1_day;
  auto days = day.count();
  res += (days / 10) + '0';
  res += (days % 10) + '0';
  return res;
}

void format_to(const DateMD &date_md, std::string &out) {
  days day = date_md.day_;
  if(day >= DateMD::DAY0901) {
    out += "09-"; day -= DateMD::DAY0901;
  } else if(day >= DateMD::DAY0801) {
    out += "08-"; day -= DateMD::DAY0801;
  } else if(day >= DateMD::DAY0701) {
    out += "07-"; day -= DateMD::DAY0701;
  } else {
    out += "06-";
  }
  day += 1_day;
  auto days = day.count();
  out += (days / 10) + '0';
  out += (days % 10) + '0';
}

DateTime DateTime::operator+(minutes minute) const {
  auto mins = time_hm_.count() + minute.count();
  auto diff = mins / MINUTES_PER_DAY;
  mins -= diff * MINUTES_PER_DAY;
  if(mins < 0) {
    mins += MINUTES_PER_DAY;
    diff -= 1;
  }
  return DateTime(DateMD(date_md_ + days(diff)), TimeHM(minutes(mins)));
}

DateTime DateTime::operator-(minutes minute) const {
  auto mins = time_hm_.count() - minute.count();
  auto diff = mins / MINUTES_PER_DAY;
  mins -= diff * MINUTES_PER_DAY;
  if(mins < 0) {
    mins += MINUTES_PER_DAY;
    diff -= 1;
  }
  return DateTime(DateMD(date_md_ + days(diff)), TimeHM(minutes(mins)));
}

DateTime& DateTime::operator+=(minutes minute) {
  return *this = *this + minute;
}

DateTime& DateTime::operator-=(minutes minute) {
  return *this = *this - minute;
}

void format_to(const DateTime &date_time, std::string &out) {
  format_to(date_time.date_md_, out);
  out += ' ';
  format_to(date_time.time_hm_, out);
}

}