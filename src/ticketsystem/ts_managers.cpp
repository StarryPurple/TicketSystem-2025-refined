#include "ts_managers.h"

namespace ticket_system {

/************* Messenger ************/
Messenger& Messenger::operator<<(const char *msg) {
  msg_ += std::string(msg);
  return *this;
}

Messenger& Messenger::operator<<(const std::string &msg) {
  msg_ += msg;
  return *this;
}

template <size_t N>
Messenger& Messenger::operator<<(const insomnia::array<char, N> &msg) {
  msg_ += msg.str();
  return *this;
}

template <class Integer> requires std::is_integral_v<Integer>
Messenger& Messenger::operator<<(Integer val) {
  msg_ += ism::itos(val);
  return *this;
}

Messenger& Messenger::append(const char *msg, size_t n) {
  for(size_t i = 0; i < n; ++i)
    msg_ += *(msg++);
  return *this;
}

/**** Parameters for all datasets ****/

inline constexpr int BUF_CAPA = 512, K_DIST = 4;

/************ UserManager ************/

UserManager::UserManager(std::filesystem::path path, Messenger &msgr)
: hash_uid_user_map_(path.append(".huid"), BUF_CAPA, K_DIST), msgr_(msgr) {}

void UserManager::AddUser(const username_t &cur_username, UserType &tar_user) {
  if(hash_uid_user_map_.empty()) {
    tar_user.access_lvl_ = HIGHEST_ACCESS_LVL;
    hash_uid_user_map_.insert(tar_user.hash(), tar_user);
    msgr_ << 0 << '\n';
    return;
  }
  access_lvl_t cur_access_lvl = 0;
  if(auto it = logged_in_users_.find(cur_username.hash());
     it == logged_in_users_.end()) {
    msgr_ << -1 << '\n';
    return;
  } else cur_access_lvl = it->second.access_lvl;
  if(cur_access_lvl <= tar_user.access_lvl_) {
    msgr_ << 0 << '\n';
    return;
  }
  hash_uid_user_map_.insert(tar_user.hash(), tar_user);
  msgr_ << 0 << '\n';
}

void UserManager::Login(const username_t &username, const password_t &password) {
  auto huid = username.hash();
  if(logged_in_users_.find(huid) != logged_in_users_.end()) {
    msgr_ << -1 << '\n';
    return;
  }
  auto it = hash_uid_user_map_.find(huid);
  if(it == hash_uid_user_map_.end()) {
    msgr_ << -1 << '\n';
    return;
  }
  auto &user = (*it).second;
  if(user.password_.hash() != password.hash()) {
    msgr_ << -1 << '\n';
    return;
  }
  logged_in_users_.emplace(huid, LoggedInUserInfo(user.access_lvl_));
  msgr_ << 0 << '\n';
}

void UserManager::Logout(const username_t &username) {
  auto huid = username.hash();
  auto it = logged_in_users_.find(huid);
  if(it == logged_in_users_.end()) {
    msgr_ << -1 << '\n';
    return;
  }
  logged_in_users_.erase(it);
  msgr_ << 0 << '\n';
}

void UserManager::QueryProfile(const username_t &cur_username, const username_t &tar_username) {
  auto chuid = cur_username.hash(), thuid = tar_username.hash();
  access_lvl_t cur_access_lvl = 0;
  if(auto it = logged_in_users_.find(cur_username.hash());
     it == logged_in_users_.end()) {
    msgr_ << -1 << '\n';
    return;
  } else cur_access_lvl = it->second.access_lvl;
  auto it = hash_uid_user_map_.find(thuid);
  if(it == hash_uid_user_map_.end()) {
    msgr_ << -1 << '\n';
    return;
  }
  auto &user = (*it).second;
  if((cur_access_lvl <= user.access_lvl_) && (chuid != thuid)) {
    msgr_ << -1 << '\n';
    return;
  }
  msgr_ << user.username_   << ' '
        << user.zh_name_    << ' '
        << user.mail_addr_  << ' '
        << user.access_lvl_ << '\n';
}

void UserManager::ModifyProfile(
  const username_t &cur_username, const username_t &tar_username,
  const password_t &password, bool has_password,
  const zh_name_t &zh_name, bool has_zh_name,
  const mail_addr_t &mail_addr, bool has_mail_addr,
  const access_lvl_t &access_lvl, bool has_access_lvl) {

  auto chuid = cur_username.hash(), thuid = tar_username.hash();
  access_lvl_t cur_access_lvl = 0;
  if(auto it = logged_in_users_.find(cur_username.hash());
     it == logged_in_users_.end()) {
    msgr_ << -1 << '\n';
    return;
     } else cur_access_lvl = it->second.access_lvl;
  auto it = hash_uid_user_map_.find(thuid);
  if(it == hash_uid_user_map_.end()) {
    msgr_ << -1 << '\n';
    return;
  }
  auto &user = (*it).second;
  if(((cur_access_lvl <= user.access_lvl_) && (chuid != thuid)) ||
     (has_access_lvl && (cur_access_lvl <= access_lvl))) {
    msgr_ << -1 << '\n';
    return;
  }
  if(has_password)   user.password_   = password;
  if(has_zh_name)    user.zh_name_    = zh_name;
  if(has_mail_addr)  user.mail_addr_  = mail_addr;
  if(has_access_lvl) user.access_lvl_ = access_lvl;
  msgr_ << user.username_   << ' '
        << user.zh_name_    << ' '
        << user.mail_addr_  << ' '
        << user.access_lvl_ << '\n';
}

bool UserManager::has_logged_in(const username_t &username) {
  return logged_in_users_.find(username.hash()) != logged_in_users_.end();
}

void UserManager::clean() {
  hash_uid_user_map_.clear();
  logged_in_users_.clear();
}

/************ TrainManager ***********/

TrainManager::TrainManager(std::filesystem::path path, Messenger &msgr)
: hash_train_id_train_map_(path.append(".htid"), BUF_CAPA, K_DIST),
  stn_hash_train_multimap_(path.append(".stn_htid"), BUF_CAPA, K_DIST),
  msgr_(msgr) {}

void TrainManager::AddTrain(const TrainType &train) {

}

void TrainManager::DeleteTrain(const train_id_t &train_id) {

}

void TrainManager::ReleaseTrain(const train_id_t &train_id) {

}

void TrainManager::QueryTrain(const train_id_t &train_id, date_md_t departure_date) {

}

void TrainManager::QueryTicket(stn_name_t departure_stn, stn_name_t arrival_stn, date_md_t departure_date, bool is_cost_order) {

}

void TrainManager::QueryTransfer(stn_name_t departure_stn, stn_name_t arrival_stn, date_md_t departure_date, bool is_cost_order) {

}

void TrainManager::get_train(const train_id_t &train_id, TrainType &train) {

}

void TrainManager::get_train(const hash_train_id_t &hash_train_id, TrainType &train) {

}

void TrainManager::update_train(const TrainType &train) {

}

void TrainManager::clean() {
  hash_train_id_train_map_.clear();
  stn_hash_train_multimap_.clear();
}

/******** TicketOrderManager *********/

TicketOrderManager::TicketOrderManager(std::filesystem::path path, Messenger &msgr)
: order_id_order_map_(path.append(".otime"), BUF_CAPA, K_DIST),
  hash_uid_order_map_(path.append(".huid_order"), BUF_CAPA, K_DIST),
  hash_train_id_order_map(path.append(".htid_order"), BUF_CAPA, K_DIST),
  order_id_allocator(path.append(".oid_alloc")),
  msgr_(msgr) {}

bool TicketOrderManager::BuyTicket(const username_t &username, TrainType &train, date_md_t departure_date, stn_name_t departure_stn, stn_name_t arrival_stn, seat_num_t ticket_num, bool accept_waitlist) {

}

void TicketOrderManager::QueryOrder(const username_t &username) {

}

ism::pair<ism::vector<TicketOrderType>, order_id_t> TicketOrderManager::RefundOrder(const username_t &username, order_id_t order_id) {

}

void TicketOrderManager::cover_ticket(TicketOrderType &order, TrainType &train) {

}

void TicketOrderManager::clean() {
  order_id_order_map_.clear();
  hash_uid_order_map_.clear();
  hash_train_id_order_map.clear();
  order_id_allocator.clear();
}

}