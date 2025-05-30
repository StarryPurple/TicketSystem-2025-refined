#ifndef TICKETSYSTEM_TS_TYPES_H
#define TICKETSYSTEM_TS_TYPES_H

#include <iostream>

#include "pair.h"
#include "array.h"
#include "ts_time.h"
#include "algorithm.h"

namespace ticket_system {

namespace ism = insomnia;

template <size_t N>
using ascii_str_t = ism::array<char, N>;
template <size_t N>
using utf8_str_t = ism::array<char, N * 4>;

/************************************************/
/*************** basic data types ***************/
/************************************************/

using username_t   = ascii_str_t<20>;
using password_t   = ascii_str_t<30>;
using zh_name_t    = utf8_str_t<5>;
using mail_addr_t  = ascii_str_t<30>;
using access_lvl_t = short;
constexpr access_lvl_t HIGHEST_ACCESS_LVL = 10;

constexpr size_t MAX_STATION_NUM = 100;
using train_id_t      = ascii_str_t<20>;
using stn_num_t       = short;
using seat_num_t      = int;
using seat_num_list_t = ism::array<seat_num_t, MAX_STATION_NUM>;
using stn_name_t      = utf8_str_t<10>;
using stn_list_t      = ism::array<stn_name_t, MAX_STATION_NUM>;
using price_t         = int;
using price_list_t    = ism::array<price_t, MAX_STATION_NUM>;
using time_hm_t       = TimeHM;
using time_dur_t      = minutes;
using dur_list_t      = ism::array<time_dur_t, MAX_STATION_NUM>;
using date_md_t       = DateMD;
using train_type_t    = char;

using order_cnt_t  = int;

/************************************************/
/**************** command types *****************/
/************************************************/

using timestamp_t = int; // timestamp type

// You can use some CRTP strategies.
template <class DerivedCmd>
struct CommandBase {
  void handle(char par_name, const char *beg, size_t n) {
    throw ism::runtime_error("Unimplemented command handler.");
  }
  void initialize(const char *ptr) {
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
  void handle(char par_name, const char *beg, size_t n);
};
struct CmdLogin : public CommandBase<CmdLogin> {
  username_t username; // -u
  password_t password; // -p
  void handle(char par_name, const char *beg, size_t n);
};
struct CmdLogout : public CommandBase<CmdLogout> {
  username_t username; // -u
  void handle(char par_name, const char *beg, size_t n);
};
struct CmdQueryProfile : public CommandBase<CmdQueryProfile> {
  username_t cur_username; // -c
  username_t tar_username; // -u
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
  void handle(char par_name, const char *beg, size_t n);
};
struct CmdDeleteTrain : public CommandBase<CmdDeleteTrain> {
  train_id_t train_id; // -i
  void handle(char par_name, const char *beg, size_t n);
};
struct CmdReleaseTrain : public CommandBase<CmdReleaseTrain> {
  train_id_t train_id; // -i
  void handle(char par_name, const char *beg, size_t n);
};
struct CmdQueryTrain : public CommandBase<CmdQueryTrain> {
  train_id_t train_id;       // -i
  date_md_t  departure_date; // -d
  void handle(char par_name, const char *beg, size_t n);
};
struct CmdQueryTicket : public CommandBase<CmdQueryTicket> {
  stn_name_t departure_stn;  // -s
  stn_name_t arrival_stn;    // -t
  date_md_t  departure_date; // -d
  bool is_cost_order;        // -p?
  void handle(char par_name, const char *beg, size_t n);
};
struct CmdQueryTransfer : public CommandBase<CmdQueryTransfer> {
  stn_name_t departure_stn;  // -s
  stn_name_t arrival_stn;    // -t
  date_md_t  departure_date; // -d
  bool is_cost_order;        // -p?
  void handle(char par_name, const char *beg, size_t n);
};
struct CmdBuyTicket : public CommandBase<CmdBuyTicket> {
  username_t username;       // -u
  train_id_t train_id;       // -i
  date_md_t  departure_date; // -d
  stn_name_t departure_stn;  // -f
  stn_name_t arrival_stn;    // -t
  seat_num_t ticket_num;     // -n
  bool accept_waitlist;      // -q?
  void handle(char par_name, const char *beg, size_t n);
};
struct CmdQueryOrder : public CommandBase<CmdQueryOrder> {
  username_t username; // -u
  void handle(char par_name, const char *beg, size_t n);
};
struct CmdRefundTicket : public CommandBase<CmdRefundTicket> {
  username_t  username;   // -u
  timestamp_t order_rank; // -n?
  void handle(char par_name, const char *beg, size_t n);
};
// struct CmdClean {};
// struct CmdExit {};

/************************************************/
/************** integrated types ****************/
/************************************************/

class UserType {
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

private:
  username_t   username_;
  password_t   password_;
  zh_name_t    zh_name_;
  mail_addr_t  mail_addr_;
  access_lvl_t access_lvl_;
};

class TrainType {
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
    const train_type_t &train_type) {

    train_id_   = train_id;
    stn_num_    = stn_num;
    stn_list_   = stn_list;
    start_time_ = start_time;
    begin_date_ = begin_date;
    final_date_ = final_date;
    train_type_ = train_type;

    for(stn_num_t i = 0; i < stn_num - 1; ++i)
      seat_num_list_[i] = seat_num;

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

  void initialize(const CmdAddTrain &cmd) {
    initialize(
      cmd.train_id, cmd.stn_num, cmd.seat_num, cmd.stn_list, cmd.price_list, cmd.start_time,
      cmd.travel_time_list, cmd.stopover_time_list, cmd.begin_date, cmd.final_date, cmd.train_type);
  }

private:
  train_id_t      train_id_;
  stn_num_t       stn_num_;
  // the number of unsold seats from the station to its next.
  // 0-based. val[stn_num - 1] is always 0.
  seat_num_list_t seat_num_list_;
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
  date_md_t       begin_date_;
  date_md_t       final_date_;
  train_type_t    train_type_;
};

class TicketOrderType {
public:

  TicketOrderType() = default;

private:
  enum class OrderStatus {
    Invalid,
    Success,
    Pending,
    Cancelled
  };

  timestamp_t order_time_;
  OrderStatus status_;
};

}

#endif