#ifndef TICKETSYSTEM_TICKETSYSTEM_H
#define TICKETSYSTEM_TICKETSYSTEM_H


#include "vector.h"
#include "unordered_map.h"
#include "ts_types.h"

namespace ticket_system {

namespace ism = insomnia;

// Only one object shall exist at one time.
// So it's fine making it a little big...
// layout problem? ignore it.
class TicketSystem {

  enum class Status {
    StatGood,
    StatShut
  };

  using hash_t = u_int64_t;

  static constexpr hash_t hash(const char *str) {
    hash_t hash = 5371;
    while(*str != '\0') {
      hash = (hash << 5) + hash + *(str++);
    }
    return hash;
  }

  static constexpr hash_t hash(const char *str, size_t n) {
    hash_t hash = 5371;
    for(size_t i = 0; i < n; ++i) {
      hash = (hash << 5) + hash + *(str++);
    }
    return hash;
  }

  using CommandFunc = void (TicketSystem::*)();

  ism::unordered_map<hash_t, CommandFunc> command_hashmap_;
  cmd_time_t timestamp_ {};
  char input_[2048] {};
  const char *token_ = input_;
  Status status_ = Status::StatGood;
  Logger logger_;

  static void to_token_end(const char *&p, char splitter = ' ') {
    while(*p != splitter && *p != '\0') ++p;
  }

  static void to_next_token(const char *&p, char splitter = ' ') {
    while(*p != splitter && *p != '\0') ++p;
    while(*p == splitter) ++p;
  }


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

  // return false if exited.
  void run();

public:

  TicketSystem();

  void work_loop() {
    do run(); while(status_ == Status::StatGood);
  }

};



}


#endif