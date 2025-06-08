#include "user_manager.h"

namespace ticket_system {

static constexpr int BUF_CAPA = 16, K_DIST = 2;

UserManager::UserManager(std::filesystem::path path, ism::Messenger &msgr)
: user_hid_user_map_(path.string() + "-huid", BUF_CAPA, K_DIST), msgr_(msgr) {}

void UserManager::AddUser(const username_t &cur_username, UserType &tar_user) {
  if(user_hid_user_map_.empty()) {
    tar_user.access_lvl_ = HIGHEST_ACCESS_LVL;
    user_hid_user_map_.insert(tar_user.hash(), tar_user);
    msgr_ << "0\n";
    return;
  }
  access_lvl_t cur_access_lvl = 0;
  if(auto it = logged_in_users_.find(cur_username.hash());
     it == logged_in_users_.end()) {
    msgr_ << "-1\n";
    return;
  } else cur_access_lvl = it->second.access_lvl;
  if(cur_access_lvl <= tar_user.access_lvl_) {
    msgr_ << "-1\n";
    return;
  }
  user_hid_user_map_.insert(tar_user.hash(), tar_user);
    msgr_ << "0\n";
}

void UserManager::Login(const username_t &username, const password_t &password) {
  auto huid = username.hash();
  if(logged_in_users_.find(huid) != logged_in_users_.end()) {
    msgr_ << "-1\n";
    return;
  }
  const auto it = user_hid_user_map_.find(huid);
  if(it == user_hid_user_map_.end()) {
    msgr_ << "-1\n";
    return;
  }
  const auto &user = (*it).second;
  if(user.password_.hash() != password.hash()) {
    msgr_ << "-1\n";
    return;
  }
  logged_in_users_.emplace(huid, LoggedInUserInfo(user.access_lvl_));
    msgr_ << "0\n";
}

void UserManager::Logout(const username_t &username) {
  auto huid = username.hash();
  const auto it = logged_in_users_.find(huid);
  if(it == logged_in_users_.end()) {
    msgr_ << "-1\n";
    return;
  }
  logged_in_users_.erase(it);
    msgr_ << "0\n";
}

void UserManager::QueryProfile(const username_t &cur_username, const username_t &tar_username) {
  auto chuid = cur_username.hash(), thuid = tar_username.hash();
  access_lvl_t cur_access_lvl = 0;
  if(auto it = logged_in_users_.find(cur_username.hash());
     it == logged_in_users_.end()) {
    msgr_ << "-1\n";
    return;
  } else cur_access_lvl = it->second.access_lvl;
  const auto it = user_hid_user_map_.find(thuid);
  if(it == user_hid_user_map_.end()) {
    msgr_ << "-1\n";
    return;
  }
  const auto &user = (*it).second;
  if((cur_access_lvl <= user.access_lvl_) && (chuid != thuid)) {
    msgr_ << "-1\n";
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
    msgr_ << "-1\n";
    return;
     } else cur_access_lvl = it->second.access_lvl;
  auto it = user_hid_user_map_.find(thuid);
  if(it == user_hid_user_map_.end()) {
    msgr_ << "-1\n";
    return;
  }
  auto &user = (*it).second;
  if((cur_access_lvl <= user.access_lvl_) && (chuid != thuid)) {
    msgr_ << "-1\n";
    return;
  }
  if(has_access_lvl && (cur_access_lvl <= access_lvl)) {
    msgr_ << "-1\n";
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
  user_hid_user_map_.clear();
  logged_in_users_.clear();
}
}