#ifndef TICKETSYSTEM_TS_TYPES_H
#define TICKETSYSTEM_TS_TYPES_H

#include <iostream>

#include "pair.h"
#include "array.h"
#include "ts_time.h"
#include "algorithm.h"
#include "messenger.h"
#include "index_pool.h"

namespace ticket_system {

namespace ism = insomnia;

template <size_t N>
using ascii_str_t = ism::array<char, N>;
template <size_t N>
using utf8_str_t = ism::array<char, N * 3>;

/************************************************/
/*************** basic data types ***************/
/************************************************/

using timestamp_t  = int; // timestamp type

using username_t   = ascii_str_t<20>;
using password_t   = ascii_str_t<30>;
using zh_name_t    = utf8_str_t<5>;
using mail_addr_t  = ascii_str_t<30>;
using access_lvl_t = short;
constexpr access_lvl_t HIGHEST_ACCESS_LVL = 10;

constexpr size_t MAX_STATION_NUM = 100;
using time_hm_t       = TimeHM;
using time_dur_t      = minutes;
using dur_list_t      = ism::array<time_dur_t, MAX_STATION_NUM>;
using date_md_t       = DateMD;
using date_time_t     = DateTime;
using train_id_t      = ascii_str_t<20>;
using stn_num_t       = short;
using seat_num_t      = int;
using seat_num_list_t = ism::array<seat_num_t, MAX_STATION_NUM>;
using stn_name_t      = utf8_str_t<10>;
using stn_list_t      = ism::array<stn_name_t, MAX_STATION_NUM>;
using price_t         = int;
using price_list_t    = ism::array<price_t, MAX_STATION_NUM>;
using train_type_t    = char;

using order_id_t      = timestamp_t;
constexpr order_id_t INVALID_ORDER_ID = ism::NULL_INDEX;

/************************************************/
/**************** command types *****************/
/************************************************/


// You can use some CRTP strategies.
template <class DerivedCmd>
struct CommandBase {
  void reset() {
    throw ism::runtime_error("Unimplemented command reset mechanic.");
  }
  void handle(char par_name, const char *beg, size_t n) {
    throw ism::runtime_error("Unimplemented command handler.");
  }
  void initialize(const char *ptr) {
    static_cast<DerivedCmd*>(this)->reset();
    while(ism::advance_past(ptr, '-')) {
      char par_name = *ptr;
      ism::advance_past(ptr, ' ');
      const char *beg = ptr;
      ism::advance_until(ptr, ' ');
      const char *end = ptr;
      static_cast<DerivedCmd*>(this)->handle(par_name, beg, end - beg);
    }
  }
};

struct CmdAddUser : public CommandBase<CmdAddUser> {
  username_t   cur_username; // -c
  username_t   tar_username; // -u
  password_t   password;     // -p
  zh_name_t    zh_name;      // -n
  mail_addr_t  mail_addr;    // -m
  access_lvl_t access_lvl;   // -g
  void reset() {}
  void handle(char par_name, const char *beg, size_t n);
};
struct CmdLogin : public CommandBase<CmdLogin> {
  username_t username; // -u
  password_t password; // -p
  void reset() {}
  void handle(char par_name, const char *beg, size_t n);
};
struct CmdLogout : public CommandBase<CmdLogout> {
  username_t username; // -u
  void reset() {}
  void handle(char par_name, const char *beg, size_t n);
};
struct CmdQueryProfile : public CommandBase<CmdQueryProfile> {
  username_t cur_username; // -c
  username_t tar_username; // -u
  void reset() {}
  void handle(char par_name, const char *beg, size_t n);
};
struct CmdModifyProfile : public CommandBase<CmdModifyProfile> {
  username_t   cur_username; // -c
  username_t   tar_username; // -u
  password_t   password;     // -p?
  zh_name_t    zh_name;      // -n?
  mail_addr_t  mail_addr;    // -m?
  access_lvl_t access_lvl;   // -g?
  bool has_password;
  bool has_zh_name;
  bool has_mail_addr;
  bool has_access_lvl;
  void reset() { has_password = has_zh_name = has_mail_addr = has_access_lvl = false; }
  void handle(char par_name, const char *beg, size_t n);
};
struct CmdAddTrain : public CommandBase<CmdAddTrain> {
  train_id_t   train_id;           // -i
  stn_num_t    stn_num;            // -n
  seat_num_t   seat_num;           // -m
  stn_list_t   stn_list;           // -s
  price_list_t price_list;         // -p
  time_hm_t    start_time;         // -x
  dur_list_t   travel_time_list;   // -t
  dur_list_t   stopover_time_list; // -o
  date_md_t    begin_date;         // -d
  date_md_t    final_date;         // -d
  train_type_t train_type;         // -y
  void reset() {}
  void handle(char par_name, const char *beg, size_t n);
};
struct CmdDeleteTrain : public CommandBase<CmdDeleteTrain> {
  train_id_t train_id; // -i
  void reset() {}
  void handle(char par_name, const char *beg, size_t n);
};
struct CmdReleaseTrain : public CommandBase<CmdReleaseTrain> {
  train_id_t train_id; // -i
  void reset() {}
  void handle(char par_name, const char *beg, size_t n);
};
struct CmdQueryTrain : public CommandBase<CmdQueryTrain> {
  train_id_t train_id;       // -i
  date_md_t  train_departure_date; // -d
  void reset() {}
  void handle(char par_name, const char *beg, size_t n);
};
struct CmdQueryTicket : public CommandBase<CmdQueryTicket> {
  stn_name_t departure_stn;  // -s
  stn_name_t arrival_stn;    // -t
  date_md_t  passenger_departure_date; // -d
  bool is_cost_order;        // -p?
  void reset() { is_cost_order = false; }
  void handle(char par_name, const char *beg, size_t n);
};
struct CmdQueryTransfer : public CommandBase<CmdQueryTransfer> {
  stn_name_t departure_stn;  // -s
  stn_name_t arrival_stn;    // -t
  date_md_t  passenger_departure_date; // -d
  bool is_cost_order;        // -p?
  void reset() { is_cost_order = false; }
  void handle(char par_name, const char *beg, size_t n);
};
struct CmdBuyTicket : public CommandBase<CmdBuyTicket> {
  username_t username;       // -u
  train_id_t train_id;       // -i
  date_md_t  passenger_departure_date; // -d
  stn_name_t from_stn;  // -f
  stn_name_t dest_stn;    // -t
  seat_num_t ticket_num;     // -n
  bool accept_waitlist;      // -q?
  void reset() { accept_waitlist = false; }
  void handle(char par_name, const char *beg, size_t n);
};
struct CmdQueryOrder : public CommandBase<CmdQueryOrder> {
  username_t username; // -u
  void reset() {}
  void handle(char par_name, const char *beg, size_t n);
};
struct CmdRefundTicket : public CommandBase<CmdRefundTicket> {
  username_t username;   // -u
  order_id_t order_rank; // -n?
  void reset() { order_rank = 1; }
  void handle(char par_name, const char *beg, size_t n);
};
// struct CmdClean {};
// struct CmdExit {};

/************************************************/
/************** integrated types ****************/
/************************************************/

using user_hid_t = ism::hash_result_t;

class UserManager;

class UserType {
  friend UserManager;

public:
  UserType() = default;

  void initialize(
    const username_t &username,
    const password_t &password, const zh_name_t &zh_name,
    const mail_addr_t &mail_addr, const access_lvl_t &access_lvl) {
    username_   = username;
    password_   = password;
    zh_name_    = zh_name;
    mail_addr_  = mail_addr;
    access_lvl_ = access_lvl;
  }

  void initialize(const CmdAddUser &cmd) {
    initialize(cmd.tar_username, cmd.password, cmd.zh_name, cmd.mail_addr, cmd.access_lvl);
  }

  [[nodiscard]]
  ism::hash_result_t hash() const { return username_.hash(); }

private:
  username_t   username_;
  password_t   password_;
  zh_name_t    zh_name_;
  mail_addr_t  mail_addr_;
  access_lvl_t access_lvl_;
};

using train_hid_t = ism::hash_result_t;
using hash_stn_name_t = ism::hash_result_t;

class TrainManager;

class TrainType {
  friend TrainManager;

public:
  TrainType() = default;
  void initialize(
    const train_id_t &train_id,
    const stn_num_t &stn_num,
    const seat_num_t &seat_num,
    const stn_list_t &stn_list,
    const price_list_t &separate_price_list,
    const time_hm_t &start_time,
    const dur_list_t &travel_time_list, const dur_list_t &stopover_time_list,
    const date_md_t &begin_date, const date_md_t &final_date,
    const train_type_t &train_type, bool has_released) {
    train_id_     = train_id;
    max_seat_num_ = seat_num;
    stn_num_      = stn_num;
    stn_list_     = stn_list;
    start_time_   = start_time;
    start_date_   = begin_date;
    final_date_   = final_date;
    train_type_   = train_type;
    has_released_ = has_released;

    accumulative_price_list_[0] = 0;
    for(stn_num_t i = 0; i < stn_num - 1; ++i)
      accumulative_price_list_[i + 1] = accumulative_price_list_[i] + separate_price_list[i];

    departure_time_list_[0] = time_dur_t(0);
    for(stn_num_t i = 0; i < stn_num - 2; ++i) {
      arrival_time_list_[i + 1] = departure_time_list_[i] + travel_time_list[i];
      departure_time_list_[i + 1] = arrival_time_list_[i + 1] + stopover_time_list[i];
    }
    arrival_time_list_[stn_num - 1] = departure_time_list_[stn_num - 2] + travel_time_list[stn_num - 2];
  }

  [[nodiscard]]
  ism::hash_result_t hash() const { return train_id_.hash(); }

  /**
   * @param train_departure_date the date the train departs from the first station.
   * @return whether this date is within the train's capability.
   */
  bool check_train_departure_date(const date_md_t &train_departure_date) const {
    return start_date_ <= train_departure_date && train_departure_date <= final_date_;
  }

  /**
   * @param passenger_departure_date the date one departs from stn[stn_ord].
   * @param stn_ord the ordinal of the station in the route of the train.
   * @return the date the train departs from the first station.
   */
  date_md_t get_train_departure_date(
    const date_md_t &passenger_departure_date, stn_num_t stn_ord) const {

    // The date_hm_t + minutes addition will automatically be normalized.
    date_time_t date_time_here(passenger_departure_date, start_time_ + departure_time_list_[stn_ord]);
    date_time_here -= departure_time_list_[stn_ord];
    return date_time_here.date_md();
  }

  price_t cost(stn_num_t from_ord, stn_num_t dest_ord) const {
    return accumulative_price_list_[dest_ord] - accumulative_price_list_[from_ord];
  }

  time_dur_t time(stn_num_t from_ord, stn_num_t dest_ord) const {
    return arrival_time_list_[dest_ord] - departure_time_list_[from_ord];
  }

private:
  train_id_t      train_id_;
  stn_num_t       stn_num_;
  // the number of unsold seats from the station to its next.
  // 0-based. val[stn_num - 1] is always 0.
  seat_num_t      max_seat_num_;
  stn_list_t      stn_list_;
  // pre-sum of prices to pay from the first station to another station.
  // 0-based. val[0] is always 0.
  price_list_t    accumulative_price_list_;
  time_hm_t       start_time_;
  // pre-sum of previous time cost before departure of a station.
  // 0-based. val[0] is always 0. val[stn_num_ - 1] is meaningless.
  dur_list_t      departure_time_list_;
  // pre-sum of previous time cost before arrival of a station.
  // 0-based. val[0] is meaningless.
  dur_list_t      arrival_time_list_;
  date_md_t       start_date_;
  date_md_t       final_date_;
  train_type_t    train_type_;
  bool            has_released_ = false;
};

class TrainSeatStatus {
  friend TrainManager;
public:

  TrainSeatStatus() = default;
  void initialize(seat_num_t seat_num, stn_num_t stn_num) {
    stn_num_ = stn_num;
     for(size_t j = 0; j < stn_num - 1; ++j)
       seat_num_list_[j] = seat_num;
  }

  seat_num_t available_seat_num(stn_num_t from_ord, stn_num_t dest_ord) const {
    seat_num_t ret = seat_num_list_[from_ord];
    for(auto i = from_ord + 1; i < dest_ord; ++i)
      ret = std::min(ret, seat_num_list_[i]);
    return ret;
  }

  void restore_seat_num(seat_num_t seat_num, stn_num_t from_ord, stn_num_t dest_ord) {
    for(auto i = from_ord; i < dest_ord; ++i)
      seat_num_list_[i] += seat_num;
  }

  void consume_seat_num(seat_num_t seat_num, stn_num_t from_ord, stn_num_t dest_ord) {
    for(auto i = from_ord; i < dest_ord; ++i)
      seat_num_list_[i] -= seat_num;
  }

private:
  seat_num_list_t seat_num_list_;
  stn_num_t stn_num_;
};

class TicketOrderManager;

class TicketOrderType {
  friend TicketOrderManager;

public:

  auto operator<=>(const TicketOrderType &other) const {
    return order_id_ <=> other.order_id_;
  }

  enum class OrderStatus {
    Invalid,
    Success,
    Pending,
    Refunded
  };

  TicketOrderType() = default;
  TicketOrderType(
    OrderStatus order_status,
    const username_t  &username,
    const train_id_t  &train_id,
    const stn_name_t  &from_stn,
    const stn_name_t  &dest_stn,
    const date_time_t &from_date_time,
    const date_time_t &dest_date_time,
    const stn_num_t   &from_stn_ord,
    const stn_num_t   &dest_stn_ord,
    const price_t     &single_price,
    const seat_num_t  &ticket_num,
    const date_md_t   &train_dep_date)
  : order_id_(INVALID_ORDER_ID), status_(order_status),
    username_(username), train_id_(train_id),
    from_stn_(from_stn), dest_stn_(dest_stn),
    from_date_time_(from_date_time), dest_date_time_(dest_date_time),
    from_stn_ord_(from_stn_ord), dest_stn_ord_(dest_stn_ord),
    single_price_(single_price), ticket_num_(ticket_num),
    train_dep_date_(train_dep_date) {}


  bool is_vaild() const { return status_ != OrderStatus::Invalid; }
  bool has_refunded() const { return status_ == OrderStatus::Refunded; }
  bool is_succeeded() const { return status_ == OrderStatus::Success; }
  bool is_pending() const { return status_ == OrderStatus::Pending; }
  void set_success() { status_ = OrderStatus::Success; }
  void set_refunded() { status_ = OrderStatus::Refunded; }
  train_id_t train_id() const { return train_id_; }
  date_md_t train_dep_date() const { return train_dep_date_; }
  seat_num_t ticket_num() const { return ticket_num_; }
  stn_num_t from_stn_ord() const { return from_stn_ord_; }
  stn_num_t dest_stn_ord() const { return dest_stn_ord_; }


  std::string string() const {
    std::string ret;
    ret.reserve(128);
    ret += '[';
    switch(status_) {
    case OrderStatus::Success:  ret += "success";  break;
    case OrderStatus::Pending:  ret += "pending";  break;
    case OrderStatus::Refunded: ret += "refunded"; break;
    default: throw ism::runtime_error("Ticket system : Invalid order status.");
    }
    ret += "] ";
    ret += train_id_.c_str(); ret += ' ';
    ret += from_stn_.c_str(); ret += ' ';
    ret += from_date_time_.string(); ret += " -> ";
    ret += dest_stn_.c_str(); ret += ' ';
    ret += dest_date_time_.string(); ret += ' ';
    ret += ism::itos(single_price_); ret += ' ';
    ret += ism::itos(ticket_num_);
    return ret;
  }

  void append_to(std::string &out) const {
    out.reserve(out.length() + 128);
    out += '[';
    switch(status_) {
    case OrderStatus::Success:  out += "success";  break;
    case OrderStatus::Pending:  out += "pending";  break;
    case OrderStatus::Refunded: out += "refunded"; break;
    default: throw ism::runtime_error("Ticket system : Invalid order status.");
    }
    out  += "] ";
    out += train_id_.c_str(); out += ' ';
    out += from_stn_.c_str(); out += ' ';
    out += from_date_time_.string(); out += " -> ";
    out += dest_stn_.c_str(); out += ' ';
    out += dest_date_time_.string(); out += ' ';
    out += ism::itos(single_price_); out += ' ';
    out += ism::itos(ticket_num_);
  }

  friend ism::Messenger& operator<<(ism::Messenger &msgr, const TicketOrderType &ticket_order) {
    ticket_order.append_to(msgr.str_ref());
    return msgr;
  }

private:

  order_id_t  order_id_;
  OrderStatus status_;
  username_t  username_;
  train_id_t  train_id_;
  stn_name_t  from_stn_;
  stn_name_t  dest_stn_;
  date_time_t from_date_time_;
  date_time_t dest_date_time_;
  stn_num_t   from_stn_ord_;
  stn_num_t   dest_stn_ord_;
  price_t     single_price_;
  seat_num_t  ticket_num_;
  date_md_t   train_dep_date_;
};

}

#endif