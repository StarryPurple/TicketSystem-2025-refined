#ifndef TICKETSYSTEM_TS_MANAGERS_H
#define TICKETSYSTEM_TS_MANAGERS_H

#include "array.h"
#include "vector.h"
#include "unordered_map.h"
#include "bplustree.h"
#include "multi_bplustree.h"
#include "ts_types.h"

namespace ticket_system {

namespace ism = insomnia;

class Messenger {
public:

  Messenger& operator<<(char msg);
  Messenger& operator<<(const char *msg);
  Messenger& operator<<(const std::string &msg);
  template<size_t N>
  Messenger& operator<<(const ism::array<char, N> &msg);
  // Messenger& operator<<(const time_dur_t &msg);
  template <class Integer> requires std::is_integral_v<Integer>
  Messenger& operator<<(Integer val);
  Messenger& append(const char *msg, size_t n);

  void print_msg() const { std::cout << msg_; }
  void print_err() const { std::cerr << msg_; }
  [[nodiscard]]
  std::string str() const { return msg_; }
  void reset() { msg_.clear(); }

private:
  std::string msg_;
};


class UserManager {

  struct LoggedInUserInfo {
    access_lvl_t access_lvl;

    LoggedInUserInfo() = default;
    explicit LoggedInUserInfo(access_lvl_t _access_lvl) : access_lvl(_access_lvl) {}
  };

public:

  UserManager(std::filesystem::path path, Messenger &msgr);
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
  Messenger &msgr_;
};

class TrainManager {
public:
  TrainManager(std::filesystem::path path, Messenger &msgr);
  ~TrainManager() = default;


  void AddTrain(const TrainType &train);
  void DeleteTrain(const train_id_t &train_id);
  void ReleaseTrain(const train_id_t &train_id);
  void QueryTrain(const train_id_t &train_id, date_md_t train_departure_date);
  void QueryTicket(
    stn_name_t from_stn, stn_name_t dest_stn,
    date_md_t passenger_departure_date,
    bool is_cost_order);
  void QueryTransfer(
    stn_name_t from_stn, stn_name_t dest_stn,
    date_md_t passenger_departure_date,
    bool is_cost_order);

  // if order can success, changes seat status immediately here.
  TicketOrderType BuyTicket(
    const username_t &username,
    const train_id_t &train_id, stn_name_t from_stn, stn_name_t dest_stn,
    date_md_t passenger_departure_date, seat_num_t ticket_num, bool accept_waitlist);
  ism::Bplustree<ism::pair<train_hid_t, days_count_t>, TrainSeatStatus>::iterator
  get_seat_status_iter(const train_id_t &train_id, date_md_t train_dep_date);
  void clean();

private:

  ism::Bplustree<train_hid_t, TrainType> train_hid_train_map_;
  ism::Bplustree<ism::pair<train_hid_t, days_count_t>, TrainSeatStatus> train_hid_seats_map_;
  // stores trains that pass this station in the form of [htid, #the ordinal of the station of the train]
  // only to be enlarged in ReleaseTrain.
  // So via this method, only released trains can be seen.
  ism::MultiBplustree<hash_stn_name_t, ism::pair<train_hid_t, stn_num_t>> stn_hid_train_info_multimap_;
  Messenger &msgr_;
};

class TicketOrderManager {

public:

  TicketOrderManager(std::filesystem::path path, Messenger &msgr);
  ~TicketOrderManager() = default;

  void record_buy_ticket(TicketOrderType &ticket_order);
  void QueryOrder(const username_t &username);

  order_id_t find_order_id(const username_t &username, order_id_t order_rank);
  ism::Bplustree<order_id_t, TicketOrderType>::iterator get_order_iter(order_id_t order_id);
  TicketOrderType get_order(order_id_t order_id);
  ism::vector<order_id_t> get_ticket_related_orders(const train_id_t &train_id, date_md_t train_dep_date);

  order_id_t new_order_id() { return order_id_allocator.alloc(); }
  void clean();

private:

  ism::Bplustree<order_id_t, TicketOrderType> order_id_order_map_;
  ism::MultiBplustree<user_hid_t, order_id_t> user_hid_order_id_map_;
  ism::MultiBplustree<ism::pair<train_hid_t, days_count_t>, order_id_t> train_hid_order_id_map_;
  ism::IndexPool order_id_allocator;
  Messenger &msgr_;
};

}

#endif