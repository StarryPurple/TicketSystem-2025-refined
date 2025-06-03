#include "ts_time.h"
#include "algorithm.h"

namespace ticket_system {

std::string TimeHM::string() const {
  std::string res;
  auto hour = count() / 60, minute = count() - 60 * hour;
  if(hour < 10) res += '0';
  res += ism::itos(hour);
  res += ':';
  if(minute < 10) res += '0';
  res += ism::itos(minute);
  return res;
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
  if(day < 10_day) res += '0';
  res += ism::itos(day.count());
  return res;
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

}