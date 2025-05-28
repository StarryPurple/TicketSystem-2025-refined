#include "ticketsystem.h"

#include "algorithm.h"

namespace ticket_system {

TicketSystem::TicketSystem() {
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
  token_ = input_; // *token = '['
  to_token_end(token_); // *token = ' '
  const char *stmp_beg = input_ + 1;
  const char *stmp_end = token_ - 1;
  timestamp_ = ism::stoi<cmd_time_t>(stmp_beg, stmp_end - stmp_beg);
  logger_.print_log(input_, token_ - input_ + 1);
  to_next_token(token_); // token = cmd name
  const char *cmd_name = token_;
  to_token_end(token_); // token = param name #1
  size_t cmd_len = token_ - cmd_name;
  to_next_token(token_);
  if(auto it = command_hashmap_.find(hash(cmd_name, cmd_len));
    it != command_hashmap_.end()) {
    (this->*it->second)();
  } else throw ism::invalid_argument(std::string("unknown command:" + std::string(cmd_name)).c_str());
}

void TicketSystem::AddUser() {

}

void TicketSystem::Login() {

}

void TicketSystem::Logout() {

}

void TicketSystem::QueryProfile() {

}

void TicketSystem::ModifyProfile() {

}

void TicketSystem::AddTrain() {

}

void TicketSystem::DeleteTrain() {

}

void TicketSystem::ReleaseTrain() {

}

void TicketSystem::QueryTrain() {

}

void TicketSystem::QueryTicket() {

}

void TicketSystem::QueryTransfer() {

}

void TicketSystem::BuyTicket() {

}

void TicketSystem::QueryOrder() {

}

void TicketSystem::RefundTicket() {

}

void TicketSystem::Clean() {

}

void TicketSystem::Exit() {
  logger_.print_log("bye\n");
  status_ = Status::StatShut;
}

}