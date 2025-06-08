#ifndef TICKETSYSTEM_USER_MANAGER_H
#define TICKETSYSTEM_USER_MANAGER_H

#include "ts_types.h"
#include "messenger.h"
#include "bplustree.h"
#include "multi_bplustree.h"

namespace ticket_system {

class UserManager {

  struct LoggedInUserInfo {
    access_lvl_t access_lvl;

    LoggedInUserInfo() = default;
    explicit LoggedInUserInfo(access_lvl_t _access_lvl) : access_lvl(_access_lvl) {}
  };

public:

  UserManager(std::filesystem::path path, ism::Messenger &msgr);
  ~UserManager() = default;

  // bool no_registered_user();

  void AddUser(const username_t &cur_username, UserType &tar_user);
  void Login(const username_t &username, const password_t &password);
  void Logout(const username_t &username);
  void QueryProfile(const username_t &cur_username, const username_t &tar_username);
  void ModifyProfile(
    const username_t &cur_username, const username_t &tar_username,
    const password_t &password, bool has_password,
    const zh_name_t &zh_name, bool has_zh_name,
    const mail_addr_t &mail_addr, bool has_mail_addr,
    const access_lvl_t &access_lvl, bool has_access_lvl);

  bool has_logged_in(const username_t &username);
  void clean();

private:

  ism::Bplustree<user_hid_t, UserType> user_hid_user_map_;
  ism::unordered_map<user_hid_t, LoggedInUserInfo> logged_in_users_;
  ism::Messenger &msgr_;
};

}

#endif