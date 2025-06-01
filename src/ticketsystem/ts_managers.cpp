#include "ts_managers.h"

namespace ticket_system {

/************* Messenger ************/

Messenger& Messenger::operator<<(char msg) {
  msg_ += msg;
  return *this;
}

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
: user_hid_user_map_(path.string() + ".huid", BUF_CAPA, K_DIST), msgr_(msgr) {}

void UserManager::AddUser(const username_t &cur_username, UserType &tar_user) {
  if(user_hid_user_map_.empty()) {
    tar_user.access_lvl_ = HIGHEST_ACCESS_LVL;
    user_hid_user_map_.insert(tar_user.hash(), tar_user);
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
  user_hid_user_map_.insert(tar_user.hash(), tar_user);
  msgr_ << 0 << '\n';
}

void UserManager::Login(const username_t &username, const password_t &password) {
  auto huid = username.hash();
  if(logged_in_users_.find(huid) != logged_in_users_.end()) {
    msgr_ << -1 << '\n';
    return;
  }
  const auto it = user_hid_user_map_.find(huid);
  if(it == user_hid_user_map_.end()) {
    msgr_ << -1 << '\n';
    return;
  }
  const auto &user = (*it).second;
  if(user.password_.hash() != password.hash()) {
    msgr_ << -1 << '\n';
    return;
  }
  logged_in_users_.emplace(huid, LoggedInUserInfo(user.access_lvl_));
  msgr_ << 0 << '\n';
}

void UserManager::Logout(const username_t &username) {
  auto huid = username.hash();
  const auto it = logged_in_users_.find(huid);
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
  const auto it = user_hid_user_map_.find(thuid);
  if(it == user_hid_user_map_.end()) {
    msgr_ << -1 << '\n';
    return;
  }
  const auto &user = (*it).second;
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
  auto it = user_hid_user_map_.find(thuid);
  if(it == user_hid_user_map_.end()) {
    msgr_ << -1 << '\n';
    return;
  }
  auto &user = (*it).second;
  if((cur_access_lvl <= user.access_lvl_) && (chuid != thuid)) {
    msgr_ << -1 << '\n';
    return;
  }
  if(has_access_lvl && (cur_access_lvl <= access_lvl)) {
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
  user_hid_user_map_.clear();
  logged_in_users_.clear();
}

/************ TrainManager ***********/

TrainManager::TrainManager(std::filesystem::path path, Messenger &msgr)
: train_hid_train_map_(path.string() + ".htid", BUF_CAPA, K_DIST),
  stn_hid_train_info_multimap_(path.string() + ".stn_htid", BUF_CAPA, K_DIST),
  msgr_(msgr) {}

void TrainManager::AddTrain(const TrainType &train) {
  auto htid = train.hash();
  if(train_hid_train_map_.find(htid) != train_hid_train_map_.end()) {
    msgr_ << -1 << '\n';
    return;
  }
  train_hid_train_map_.insert(htid, train);
  msgr_ << 0 << '\n';
}

void TrainManager::DeleteTrain(const train_id_t &train_id) {
  auto htid = train_id.hash();
  const auto it = train_hid_train_map_.find(htid);
  if(it == train_hid_train_map_.end()) {
    msgr_ << -1 << '\n';
    return;
  }
  if(const auto &train = (*it).second; train.has_released_) {
    msgr_ << -1 << '\n';
    return;
  }
  train_hid_train_map_.remove(htid);
  msgr_ << 0 << '\n';
}

void TrainManager::ReleaseTrain(const train_id_t &train_id) {
  auto htid = train_id.hash();
  auto it = train_hid_train_map_.find(htid);
  if(it == train_hid_train_map_.end()) {
    msgr_ << -1 << '\n';
    return;
  }
  if(const auto &train = it.view().second; train.has_released_) {
    msgr_ << -1 << '\n';
    return;
  }
  auto &train = (*it).second;
  train.has_released_ = true;
  for(stn_num_t i = 0; i < train.stn_num_; ++i)
    stn_hid_train_info_multimap_.insert(
      train.stn_list_[i].hash(), ism::pair<train_hid_t, stn_num_t>(htid, i));
  msgr_ << 0 << '\n';
}

void TrainManager::QueryTrain(const train_id_t &train_id, date_md_t train_departure_date) {
  auto htid = train_id.hash();
  const auto it = train_hid_train_map_.find(htid);
  if(it == train_hid_train_map_.end()) {
    msgr_ << -1 << '\n';
    return;
  }
  const auto &train = (*it).second;
  if(!train.check_train_departure_date(train_departure_date)) {
    msgr_ << -1 << '\n';
    return;
  }
  // train info
  msgr_ << train.train_id_ << ' ' << train.train_type_ << '\n';
  const date_time_t date_time(train_departure_date, train.start_time_);
  for(size_t i = 0; i < train.stn_num_; ++i) {
    msgr_ << train.stn_list_[i] << ' ';
    if(i == 0)
      msgr_ << date_time_t::default_string();
    else {
      msgr_ << date_time_t(date_time + train.arrival_time_list_[i]).string();
    }
    msgr_ << " -> ";
    if(i == train.stn_num_ - 1)
      msgr_ << date_time_t::default_string();
    else {
      msgr_ << date_time_t(date_time + train.departure_time_list_[i]).string();
    }
    msgr_ << ' ' << train.accumulative_price_list_[i] << ' ';
    if(i == train.stn_num_ - 1)
      msgr_ << 'x' << '\n';
    else
      msgr_ << train.seat_num_list_[i] << '\n';
  }
}

void TrainManager::QueryTicket(
  stn_name_t from_stn, stn_name_t dest_stn,
  date_md_t passenger_departure_date, bool is_cost_order) {

  auto from_hsid = from_stn.hash();
  auto dest_hsid   = dest_stn.hash();

  // already sorted in terms of htid.
  const auto from_train_list = stn_hid_train_info_multimap_.search(from_hsid);
  const auto dest_train_list = stn_hid_train_info_multimap_.search(dest_hsid);

  const auto from_train_num = from_train_list.size();
  const auto dest_train_num = dest_train_list.size();


  ism::vector<QueryResultType> ret_vec;

  for(size_t from_ptr = 0, dest_ptr = 0;
      from_ptr < from_train_num && dest_ptr < dest_train_num; ) {
    const auto &[from_train_hid, from_ord] = from_train_list[from_ptr];
    const auto &[dest_train_hid, dest_ord] = dest_train_list[dest_ptr];
    if(from_train_hid < dest_train_hid) { ++from_ptr; continue; }
    if(from_train_hid > dest_train_hid) { ++dest_ptr; continue; }
    if(from_ord > dest_ord) { ++from_ptr; ++dest_ptr; continue; }

    const auto it = train_hid_train_map_.find(dest_train_hid);
    const auto &train = (*it).second;

    // check: if the train covers the passenger departure date.
    auto train_departure_date = train.get_train_departure_date(passenger_departure_date, from_ord);
    if(!train.check_train_departure_date(train_departure_date)) { ++from_ptr; ++dest_ptr; continue; }

    date_time_t train_departure_date_time(train_departure_date, train.start_time_);
    auto time = train.arrival_time_list_[dest_ord] - train.departure_time_list_[from_ord];
    auto cost = train.accumulative_price_list_[dest_ord] - train.accumulative_price_list_[from_ord];
    auto available_seat_num = train.available_seat_num(from_ord, dest_ord);
    Messenger tmp_msgr;
    tmp_msgr << train.train_id_ << ' ' << from_stn << ' '
             << date_time_t(train_departure_date_time + train.departure_time_list_[from_ord]).string()
             << " -> " << dest_stn << ' '
             << date_time_t(train_departure_date_time + train.arrival_time_list_[dest_ord]).string()
             << ' ' << cost << ' ' << available_seat_num << '\n';
    ret_vec.emplace_back(tmp_msgr.str(), time, cost);
    ++from_ptr; ++dest_ptr;
  }

  ism::sort(
    ret_vec.begin(), ret_vec.end(),
    [is_cost_order](const QueryResultType &A, const QueryResultType &B) {
      return is_cost_order ? (A.cost < B.cost) : (A.time < B.time);
    });
  msgr_ << ret_vec.size() << '\n';
  for(const auto& ret : ret_vec)
    msgr_ << ret.msg;
}

void TrainManager::QueryTransfer(
  stn_name_t from_stn, stn_name_t dest_stn,
  date_md_t passenger_departure_date, bool is_cost_order) {

}

void TrainManager::get_train(const train_id_t &train_id, TrainType &train) {

}

void TrainManager::get_train(const train_hid_t &hash_train_id, TrainType &train) {

}

void TrainManager::update_train(const TrainType &train) {

}

void TrainManager::clean() {
  train_hid_train_map_.clear();
  stn_hid_train_info_multimap_.clear();
}

/******** TicketOrderManager *********/

TicketOrderManager::TicketOrderManager(std::filesystem::path path, Messenger &msgr)
: order_id_order_map_(path.string() + ".otime", BUF_CAPA, K_DIST),
  user_hid_order_map_(path.string() + ".huid_order", BUF_CAPA, K_DIST),
  train_hid_order_map(path.string() + ".htid_order", BUF_CAPA, K_DIST),
  order_id_allocator(path.string() + ".oid_alloc"),
  msgr_(msgr) {}

bool TicketOrderManager::BuyTicket(const username_t &username, TrainType &train, date_md_t departure_date, stn_name_t departure_stn, stn_name_t arrival_stn, seat_num_t ticket_num, bool accept_waitlist) {

}

void TicketOrderManager::QueryOrder(const username_t &username) {

}

ism::pair<ism::vector<TicketOrderType>, order_id_t> TicketOrderManager::RefundOrder(const username_t &username, order_id_t order_id) {

}

bool TicketOrderManager::try_cover(TicketOrderType &order, TrainType &train) {

}

void TicketOrderManager::clean() {
  order_id_order_map_.clear();
  user_hid_order_map_.clear();
  train_hid_order_map.clear();
  order_id_allocator.clear();
}

}