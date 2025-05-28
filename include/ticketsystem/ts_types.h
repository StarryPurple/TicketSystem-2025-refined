#ifndef TICKETSYSTEM_TS_TYPES_H
#define TICKETSYSTEM_TS_TYPES_H

#include <iostream>

#include "pair.h"
#include "array.h"
#include "ts_time.h"

namespace ticket_system {

namespace ism = insomnia;

template <size_t N>
using en_str_t = ism::array<char, N>;
template <size_t N>
using zh_str_t = ism::array<char, N * 4>;

using username_t   = en_str_t<20>;
using password_t   = en_str_t<30>;
using zh_name_t    = zh_str_t<5>;
using mail_addr_t  = en_str_t<30>;
using access_lvl_t = short;


constexpr size_t MAX_STATION_NUM = 100;
using train_id_t   = en_str_t<20>;
using stn_num_t    = short;
using seat_num_t   = int;
using stn_name_t   = zh_str_t<10>;
using stn_list_t   = ism::array<stn_name_t, MAX_STATION_NUM>;
using price_t      = int;
using price_list_t = ism::array<price_t, MAX_STATION_NUM>;
using time_hm_t    = TimeHM;
using time_dur_t   = minutes;
using dur_list_t   = ism::array<time_dur_t, MAX_STATION_NUM>;
using date_md_t    = DateMD;
using train_type_t = char;

using order_cnt_t  = int;

/*
// frequency: SF ~ 1e6, F ~ 1e5, N ~ 1e4, R ~ 1e2
enum class CommandType {
  Invalid,
  ADD_USER,       // N
  LOGIN,          // F
  LOGOUT,         // F
  QUERY_PROFILE,  // SF
  MODIFY_PROFILE, // F
  ADD_TRAIN,      // N
  DELETE_TRAIN,   // N
  RELEASE_TRAIN,  // N
  QUERY_TRAIN,    // N
  QUERY_TICKET,   // SF
  QUERY_TRANSFER, // N
  BUY_TICKET,     // SF
  QUERY_ORDER,    // F
  REFUND_TICKET,  // N
  CLEAN,          // R
  EXIT            // R
};
*/

using cmd_time_t = int; // timestamp type

struct CmdAddUser {
  username_t   cur_username;
  username_t   tar_username;
  password_t   password;
  zh_name_t    zh_name;
  mail_addr_t  mail_addr;
  access_lvl_t access_lvl;
};
struct CmdLogin {
  username_t username;
  password_t password;
};
struct CmdLogout {
  username_t username;
};
struct CmdQueryProfile {
  username_t cur_username;
  username_t tar_username;
};
struct CmdModifyProfile {
  username_t cur_username;
  username_t tar_username;
  password_t   password;
  zh_name_t    zh_name;
  mail_addr_t  mail_addr;
  access_lvl_t access_lvl;
  bool has_password;
  bool has_zh_name;
  bool has_mail_addr;
  bool has_access_lvl;
};
struct CmdAddTrain {
  train_id_t   train_id;
  stn_num_t    stn_num;
  seat_num_t   seat_num;
  stn_list_t   stn_list;
  price_list_t price_list;
  time_hm_t    start_time;
  dur_list_t   travel_time_list;
  dur_list_t   stopover_time_list;
  date_md_t    begin_date;
  date_md_t    final_date;
  train_type_t train_type;
};
struct CmdDeleteTrain {
  train_id_t train_id;
};
struct CmdReleaseTrain {
  train_id_t train_id;
};
struct CmdQueryTrain {
  date_md_t  departure_date;
  train_id_t train_id;
};
struct CmdQueryTicket {
  date_md_t  departure_date;
  stn_name_t departure_stn;
  stn_name_t arrival_stn;
  bool is_cost_order;
};
struct CmdQueryTransfer {
  date_md_t  departure_date;
  stn_name_t departure_stn;
  stn_name_t arrival_stn;
  bool is_cost_order;
};
struct CmdBuyTicket {
  username_t username;
  train_id_t train_id;
  date_md_t  departure_date;
  stn_name_t departure_stn;
  stn_name_t arrival_stn;
  seat_num_t ticket_num;
  bool accept_waitlist;
};
struct CmdQueryOrder {
  username_t username;
};
struct CmdRefundTicket {
  username_t username;
  cmd_time_t order_rank;
};
// struct CmdClean {};
// struct CmdExit {};

class Logger {

public:

  void print_log(const char *log) {
    size_t n = strlen(log);
    for(size_t i = 0; i < n; ++i)
      putchar(*(log++));
  }

  void print_log(const char *log, size_t n) {
    for(size_t i = 0; i < n; ++i) putchar(*(log++));
  }

};

}

#endif