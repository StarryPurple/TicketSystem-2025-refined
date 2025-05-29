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

using username_t   = ascii_str_t<20>;
using password_t   = ascii_str_t<30>;
using zh_name_t    = utf8_str_t<5>;
using mail_addr_t  = ascii_str_t<30>;
using access_lvl_t = short;


constexpr size_t MAX_STATION_NUM = 100;
using train_id_t   = ascii_str_t<20>;
using stn_num_t    = short;
using seat_num_t   = int;
using stn_name_t   = utf8_str_t<10>;
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

// You can use some CRTP strategies.
struct CommandBase {
  CommandBase() = default;
  virtual ~CommandBase() = default;

  virtual void handle(char par_name, const char *beg, size_t n) = 0;
  void initialize(const char *ptr) {
    while(ism::advance_past(ptr, '-')) {
      char par_name = *ptr;
      ism::advance_past(ptr, ' ');
      const char *beg = ptr;
      ism::advance_until(ptr, ' ');
      const char *end = ptr;
      handle(par_name, beg, end - beg);
    }
  }
};

struct CmdAddUser : public CommandBase {
  username_t   cur_username; // -c
  username_t   tar_username; // -u
  password_t   password;     // -p
  zh_name_t    zh_name;      // -n
  mail_addr_t  mail_addr;    // -m
  access_lvl_t access_lvl;   // -g
  void handle(char par_name, const char *beg, size_t n) override;
};
struct CmdLogin : public CommandBase {
  username_t username; // -u
  password_t password; // -p
  void handle(char par_name, const char *beg, size_t n) override;
};
struct CmdLogout : public CommandBase {
  username_t username; // -u
  void handle(char par_name, const char *beg, size_t n) override;
};
struct CmdQueryProfile : public CommandBase {
  username_t cur_username; // -c
  username_t tar_username; // -u
  void handle(char par_name, const char *beg, size_t n) override;
};
struct CmdModifyProfile : public CommandBase {
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
  void handle(char par_name, const char *beg, size_t n) override;
};
struct CmdAddTrain : public CommandBase {
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
  void handle(char par_name, const char *beg, size_t n) override;
};
struct CmdDeleteTrain : public CommandBase {
  train_id_t train_id; // -i
  void handle(char par_name, const char *beg, size_t n) override;
};
struct CmdReleaseTrain : public CommandBase {
  train_id_t train_id; // -i
  void handle(char par_name, const char *beg, size_t n) override;
};
struct CmdQueryTrain : public CommandBase {
  train_id_t train_id;       // -i
  date_md_t  departure_date; // -d
  void handle(char par_name, const char *beg, size_t n) override;
};
struct CmdQueryTicket : public CommandBase {
  stn_name_t departure_stn;  // -s
  stn_name_t arrival_stn;    // -t
  date_md_t  departure_date; // -d
  bool is_cost_order;        // -p?
  void handle(char par_name, const char *beg, size_t n) override;
};
struct CmdQueryTransfer : public CommandBase {
  stn_name_t departure_stn;  // -s
  stn_name_t arrival_stn;    // -t
  date_md_t  departure_date; // -d
  bool is_cost_order;        // -p?
  void handle(char par_name, const char *beg, size_t n) override;
};
struct CmdBuyTicket : public CommandBase {
  username_t username;       // -u
  train_id_t train_id;       // -i
  date_md_t  departure_date; // -d
  stn_name_t departure_stn;  // -f
  stn_name_t arrival_stn;    // -t
  seat_num_t ticket_num;     // -n
  bool accept_waitlist;      // -q?
  void handle(char par_name, const char *beg, size_t n) override;
};
struct CmdQueryOrder : public CommandBase {
  username_t username; // -u
  void handle(char par_name, const char *beg, size_t n) override;
};
struct CmdRefundTicket : public CommandBase {
  username_t username;   // -u
  cmd_time_t order_rank; // -n?
  void handle(char par_name, const char *beg, size_t n) override;
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

  void error_log(const char *log) {
    std::cerr << log;
  }

};

}

#endif