#include "ticketsystem.h"

#include "algorithm.h"

namespace ticket_system {

TicketSystem::TicketSystem(std::filesystem::path path)
: user_mgr_(path.assign(".user"), &msgr_), train_mgr_(path.assign(".train"), &msgr_),
  order_mgr_(path.assign(".order"), &msgr_) {
  command_hashmap_[hash("add_user")]       = &TicketSystem::AddUser;
  command_hashmap_[hash("login")]          = &TicketSystem::Login;
  command_hashmap_[hash("logout")]         = &TicketSystem::Logout;
  command_hashmap_[hash("query_profile")]  = &TicketSystem::QueryProfile;
  command_hashmap_[hash("modify_profile")] = &TicketSystem::ModifyProfile;
  command_hashmap_[hash("add_train")]      = &TicketSystem::AddTrain;
  command_hashmap_[hash("delete_train")]   = &TicketSystem::DeleteTrain;
  command_hashmap_[hash("release_train")]  = &TicketSystem::ReleaseTrain;
  command_hashmap_[hash("query_train")]    = &TicketSystem::QueryTrain;
  command_hashmap_[hash("query_ticket")]   = &TicketSystem::QueryTicket;
  command_hashmap_[hash("query_transfer")] = &TicketSystem::QueryTransfer;
  command_hashmap_[hash("buy_ticket")]     = &TicketSystem::BuyTicket;
  command_hashmap_[hash("query_order")]    = &TicketSystem::QueryOrder;
  command_hashmap_[hash("refund_ticket")]  = &TicketSystem::RefundTicket;
  command_hashmap_[hash("clean")]          = &TicketSystem::Clean;
  command_hashmap_[hash("exit")]           = &TicketSystem::Exit;
}

void TicketSystem::run() {
  std::cin.getline(input_, sizeof(input_));
  if(strlen(input_) == 0) return;
  token_ = input_;                 // *token = '['
  ism::advance_until(token_, ' '); // *token = ' '
  const char *stmp_beg = input_ + 1;
  const char *stmp_end = token_ - 1;
  timestamp_ = ism::stoi<timestamp_t>(stmp_beg, stmp_end - stmp_beg);
  msgr_.append(input_, token_ - input_ + 1); // with the ' ' .
  ism::advance_past(token_, ' ');  // token = cmd name
  const char *cmd_name = token_;
  ism::advance_until(token_, ' '); // token = ' ' or '\0'
  size_t cmd_len = token_ - cmd_name;
  if(auto it = command_hashmap_.find(hash(cmd_name, cmd_len));
    it != command_hashmap_.end()) {
    (this->*it->second)();
  } else throw ism::invalid_argument(std::string("unknown command:" + std::string(cmd_name)).c_str());
  msgr_.print_msg();
  msgr_.reset();
}

void TicketSystem::AddUser() {
  static CmdAddUser cmd;
  cmd.initialize(input_);
  if(user_mgr_.NoRegisteredUserCheck()) {
    cmd.access_lvl = HIGHEST_ACCESS_LVL;
  }
}

void TicketSystem::Login() {
  static CmdLogin cmd;
  cmd.initialize(input_);
}

void TicketSystem::Logout() {
  static CmdLogout cmd;
  cmd.initialize(input_);
}

void TicketSystem::QueryProfile() {
  static CmdQueryProfile cmd;
  cmd.initialize(input_);
}

void TicketSystem::ModifyProfile() {
  static CmdModifyProfile cmd;
  cmd.initialize(input_);
}

void TicketSystem::AddTrain() {
  static CmdAddTrain cmd;
  cmd.initialize(input_);
  static TrainType train;
  train.initialize(cmd);
}

void TicketSystem::DeleteTrain() {
  static CmdDeleteTrain cmd;
  cmd.initialize(input_);
}

void TicketSystem::ReleaseTrain() {
  static CmdReleaseTrain cmd;
  cmd.initialize(input_);
}

void TicketSystem::QueryTrain() {
  static CmdQueryTrain cmd;
  cmd.initialize(input_);
}

void TicketSystem::QueryTicket() {
  static CmdQueryTicket cmd;
  cmd.initialize(input_);
}

void TicketSystem::QueryTransfer() {
  static CmdQueryTransfer cmd;
  cmd.initialize(input_);
}

void TicketSystem::BuyTicket() {
  static CmdBuyTicket cmd;
  cmd.initialize(input_);
}

void TicketSystem::QueryOrder() {
  static CmdQueryOrder cmd;
  cmd.initialize(input_);
}

void TicketSystem::RefundTicket() {
  static CmdRefundTicket cmd;
  cmd.initialize(input_);
}

void TicketSystem::Clean() {
  user_mgr_.clean();
  train_mgr_.clean();
  order_mgr_.clean();
  msgr_ << "0\n";
}

void TicketSystem::Exit() {
  msgr_ << "bye\n";
  system_status_ = SystemStatus::StatShut;
}

}