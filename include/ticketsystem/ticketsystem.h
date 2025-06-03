#ifndef TICKETSYSTEM_TICKETSYSTEM_H
#define TICKETSYSTEM_TICKETSYSTEM_H


#include "ts_managers.h"

namespace ticket_system {

namespace ism = insomnia;

// Only one object shall exist at one time.
// So it's fine making it a little big...
// layout problem? ignore it.
class TicketSystem {

public:
  explicit TicketSystem(std::filesystem::path path);
  void work_loop() {
    do run(); while(system_status_ == SystemStatus::StatGood);
    msgr_.flush(); // Huh.
  }

private:

  static constexpr ism::hash_result_t hash(const char *str) {
    return ism::hash<const char*>()(str);
  }

  static constexpr ism::hash_result_t hash(const char *str, size_t n) {
    return ism::hash<const char*>()(str, n);
  }

  enum class SystemStatus {
    StatGood,
    StatEnd
  };

  using CommandFunc = void (TicketSystem::*)();

  ism::unordered_map<ism::hash_result_t, CommandFunc> command_hashmap_;
  timestamp_t timestamp_ {};
  char input_[16384] {};
  const char *token_ = input_;
  SystemStatus system_status_ = SystemStatus::StatGood;
  Messenger msgr_;
  UserManager user_mgr_;
  TrainManager train_mgr_;
  TicketOrderManager order_mgr_;

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
};



}


#endif