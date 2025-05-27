#ifndef TICKETSYSTEM_TICKETSYSTEM_H
#define TICKETSYSTEM_TICKETSYSTEM_H

#include <iostream>
#include <optional.h>

#include "vector.h"
#include "algorithm.h"
#include "ts_types.h"

namespace ticket_system {

namespace ism = insomnia;

// Only one object shall exist at one time.
// So it's fine making it a little big...
// layout problem? ignore it.
class TicketSystem {
  CommandType type {};
  cmd_time_t timestamp {};
  union {
    CmdAddUser       cmd_add_user;
    CmdLogin         cmd_login;
    CmdLogout        cmd_logout;
    CmdQueryProfile  cmd_query_profile;
    CmdModifyProfile cmd_modify_profile;
    CmdAddTrain      cmd_add_train;
    CmdDeleteTrain   cmd_delete_train;
    CmdReleaseTrain  cmd_release_train;
    CmdQueryTrain    cmd_query_train;
    CmdQueryTicket   cmd_query_ticket;
    CmdQueryTransfer cmd_query_transfer;
    CmdBuyTicket     cmd_buy_ticket;
    CmdQueryOrder    cmd_query_order;
    CmdRefundTicket  cmd_refund_ticket;
    // CmdClean         cmd_clean;
    // CmdExit          cmd_exit;
  } cmd_union {};
  char input[2048] {};

  static void to_token_end(const char *&p, char splitter = ' ') {
    while(*p != splitter) ++p;
  }

  static void to_next_token(const char *&p, char splitter = ' ') {
    while(*p != splitter) ++p;
    while(*p == splitter) ++p;
  }

  void get_command_type(const char *str);

  void AddUser();
  void Login();
  void Logout();
  void QueryProfile();
  void ModifyProfile();
  void AddTrain();
  void DeleteTrain();
  void ReleaseTrain();
  void QueryTrain();
  void QueryTicket();
  void QueryTransfer();
  void BuyTicket();
  void QueryOrder();
  void RefundTicket();
  void Clean();
  void Exit();

public:
  void read() {
    std::cin.getline(input, sizeof(input));
    const char *token = input + 1;
    timestamp = ism::stoi<cmd_time_t>(token);
    to_next_token(token);
    get_command_type(token);
    to_next_token(token);
  }
};



}


#endif