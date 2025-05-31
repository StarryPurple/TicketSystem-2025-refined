#ifndef TICKETSYSTEM_TS_MANAGERS_H
#define TICKETSYSTEM_TS_MANAGERS_H

#include "vector.h"
#include "unordered_map.h"
#include "bplustree.h"
#include "multi_bplustree.h"
#include "ts_types.h"

namespace ticket_system {

namespace ism = insomnia;


class Messenger {
public:

  Messenger& operator<<(const char *msg) {
    msg_ += std::string(msg);
    return *this;
  }
  Messenger& operator<<(const std::string &msg) {
    msg_ += msg;
    return *this;
  }
  template <class Integer>
    requires std::is_integral_v<Integer>
  Messenger& operator<<(Integer val) {
    msg_ += ism::itos(val);
    return *this;
  }

  Messenger& append(const char *msg, size_t n) {
    for(size_t i = 0; i < n; ++i)
      msg_ += *(msg++);
    return *this;
  }

  void print_msg() const { std::cout << msg_; }
  void print_err() const { std::cerr << msg_; }
  [[nodiscard]]
  std::string str() const { return msg_; }
  void reset() { msg_.clear(); }

private:
  std::string msg_;
};

using hash_uid_t = ism::hash_result_t;

class UserManager {
public:

  UserManager(std::filesystem::path path, Messenger *msgr)
    : hash_uid_user_map_(path.assign(".huid"), BUFFER_CAPA, K_DIST), msgr_(msgr) {}
  ~UserManager() = default;

  bool NoRegisteredUserCheck();
  static bool UserAccessCheck(const access_lvl_t &cur_access_lvl, const access_lvl_t &tar_access_lvl);
  bool UserLoggedInCheck(const username_t &username);

  void AddUser(const UserType &user);
  void Login(const username_t &username);
  void Logout(const username_t &username);
  void QueryProfile(const username_t &username);
  void ModifyProfile(
    const password_t &password, bool has_password,
    const zh_name_t &zh_name, bool has_zh_name,
    const mail_addr_t &mail_addr, bool has_mail_addr,
    const access_lvl_t &access_lvl, bool has_access_lvl);

  bool BuyTicket(
    train_id_t train_id,
    date_md_t departure_date,
    stn_name_t departure_stn, stn_name_t arrival_stn,
    seat_num_t ticket_num,
    bool accept_waitlist);
  ism::vector<timestamp_t> QueryOrder();
  bool RefundTicket(timestamp_t order_rank);

  void clean();

private:
  static constexpr int BUFFER_CAPA = 512, K_DIST = 4;

  struct LoggedInUserInfo {
    access_lvl_t access_lvl;
  };

  ism::Bplustree<hash_uid_t, UserType> hash_uid_user_map_;
  ism::unordered_map<hash_uid_t, LoggedInUserInfo> logged_in_users_;
  Messenger *msgr_;
};

using hash_train_id_t = ism::hash_result_t;

class TrainManager {
public:
  TrainManager(std::filesystem::path path, Messenger *msgr)
    : hash_train_id_train_map_(path.assign(".htid"), BUFFER_CAPA, K_DIST),
      stn_hash_train_multimap_(path.assign(".stn_htid"), BUFFER_CAPA, K_DIST),
      msgr_(msgr) {}
  ~TrainManager() = default;

  void AddTrain(const TrainType &train);
  void DeleteTrain(const train_id_t &train_id);
  void ReleaseTrain(const train_id_t &train_id);
  void QueryTrain(const train_id_t &train_id, date_md_t departure_date);
  void QueryTicket(
    stn_name_t departure_stn, stn_name_t arrival_stn,
    date_md_t departure_date,
    bool is_cost_order);
  void QueryTransfer(
    stn_name_t departure_stn, stn_name_t arrival_stn,
    date_md_t departure_date,
    bool is_cost_order);

  void clean();

private:
  static constexpr int BUFFER_CAPA = 512, K_DIST = 4;

  ism::Bplustree<hash_train_id_t, TrainType> hash_train_id_train_map_;
  ism::MultiBplustree<ism::hash_result_t, TrainType> stn_hash_train_multimap_;
  Messenger *msgr_;
};

class TicketOrderManager {

public:
  TicketOrderManager(std::filesystem::path path, Messenger *msgr)
    : otime_order_map_(path.assign(".otime"), BUFFER_CAPA, K_DIST),
      hash_train_id_order_map(path.assign(".htid_order"), BUFFER_CAPA, K_DIST),
      msgr_(msgr) {}
  ~TicketOrderManager() = default;

  void BuyTicket(timestamp_t timestamp);
  void QueryOrder(ism::vector<timestamp_t> &order_list);
  void RefundOrder(timestamp_t timestamp);

  void clean();

private:
  static constexpr int BUFFER_CAPA = 512, K_DIST = 4;

  ism::Bplustree<timestamp_t, TicketOrderType> otime_order_map_;
  ism::MultiBplustree<hash_train_id_t, TicketOrderType> hash_train_id_order_map;
  Messenger *msgr_;
};

}

#endif