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

inline constexpr int BUF_CAPA = 512, K_DIST = 4;

class UserManager {

  struct LoggedInUserInfo {
    access_lvl_t access_lvl;
  };

public:

  UserManager(std::filesystem::path path, Messenger *msgr)
    : hash_uid_user_map_(path.assign(".huid"), BUF_CAPA, K_DIST), msgr_(msgr) {}
  ~UserManager() = default;

  // bool no_registered_user();

  void AddUser(const username_t &cur_username, UserType &user);
  void Login(const username_t &username, const password_t &password);
  void Logout(const username_t &username);
  void QueryProfile(const username_t &cur_username, const username_t &tar_username);
  void ModifyProfile(
    const username_t &cur_username, const username_t &tar_username,
    const password_t &password, bool has_password,
    const zh_name_t &zh_name, bool has_zh_name,
    const mail_addr_t &mail_addr, bool has_mail_addr,
    const access_lvl_t &access_lvl, bool has_access_lvl);

  ism::vector<timestamp_t> QueryOrder();
  bool RefundTicket(timestamp_t order_rank);

  bool has_logged_in(const username_t &username);
  void clean();

private:

  ism::Bplustree<hash_uid_t, UserType> hash_uid_user_map_;
  ism::unordered_map<hash_uid_t, LoggedInUserInfo> logged_in_users_;
  Messenger *msgr_;
};

class TrainManager {
public:
  TrainManager(std::filesystem::path path, Messenger *msgr)
    : hash_train_id_train_map_(path.assign(".htid"), BUF_CAPA, K_DIST),
      stn_hash_train_multimap_(path.assign(".stn_htid"), BUF_CAPA, K_DIST),
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

  void get_train(const train_id_t &train_id, TrainType &train);
  void get_train(const hash_train_id_t &hash_train_id, TrainType &train);
  void update_train(const TrainType &train);
  void clean();

private:

  ism::Bplustree<hash_train_id_t, TrainType> hash_train_id_train_map_;
  ism::MultiBplustree<ism::hash_result_t, TrainType> stn_hash_train_multimap_;
  Messenger *msgr_;
};

class TicketOrderManager {

public:

  TicketOrderManager(std::filesystem::path path, Messenger *msgr)
    : otime_order_map_(path.assign(".otime"), BUF_CAPA, K_DIST),
      hash_uid_order_map_(path.assign(".huid_order"), BUF_CAPA, K_DIST),
      hash_train_id_order_map(path.assign(".htid_order"), BUF_CAPA, K_DIST),
      msgr_(msgr) {}
  ~TicketOrderManager() = default;

  bool BuyTicket(
    const username_t &username, TrainType &train,
    date_md_t departure_date,
    stn_name_t departure_stn, stn_name_t arrival_stn,
    seat_num_t ticket_num,
    bool accept_waitlist);
  void QueryOrder(const username_t &username);
  // returns the affected_orders and the position of this order in the list.
  ism::pair<ism::vector<TicketOrderType>, order_id_t>
  RefundOrder(const username_t &username, order_id_t order_id);

  void cover_ticket(TicketOrderType &order, TrainType &train);

  void clean();

private:

  ism::Bplustree<order_id_t, TicketOrderType> otime_order_map_;
  ism::MultiBplustree<hash_uid_t, TicketOrderType> hash_uid_order_map_;
  ism::MultiBplustree<hash_train_id_t, TicketOrderType> hash_train_id_order_map;
  Messenger *msgr_;
};

}

#endif