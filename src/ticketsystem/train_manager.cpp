#include "train_manager.h"

namespace ticket_system {

static constexpr int BUF_CAPA = 192, K_DIST = 2;

TrainManager::TrainManager(std::filesystem::path path, ism::Messenger &msgr)
: train_hid_train_map_(path.string() + "-htid", BUF_CAPA, K_DIST),
  train_hid_seats_map_(path.string() + "-htid_seats", BUF_CAPA, K_DIST),
  stn_hid_train_info_multimap_(path.string() + "-stn_htid", BUF_CAPA, K_DIST),
  msgr_(msgr) {}

void TrainManager::AddTrain(const TrainType &train) {
  auto htid = train.hash();
  if(train_hid_train_map_.find(htid) != train_hid_train_map_.end()) {
    msgr_ << "-1\n";
    return;
  }
  train_hid_train_map_.insert(htid, train);
    msgr_ << "0\n";
}

void TrainManager::DeleteTrain(const train_id_t &train_id) {
  auto htid = train_id.hash();
  const auto it = train_hid_train_map_.find(htid);
  if(it == train_hid_train_map_.end()) {
    msgr_ << "-1\n";
    return;
  }
  if(const auto &train = (*it).second; train.has_released_) {
    msgr_ << "-1\n";
    return;
  }
  train_hid_train_map_.remove(htid);
    msgr_ << "0\n";
}

void TrainManager::ReleaseTrain(const train_id_t &train_id) {
  auto htid = train_id.hash();
  auto it = train_hid_train_map_.find(htid);
  if(it == train_hid_train_map_.end()) {
    msgr_ << "-1\n";
    return;
  }
  if(const auto &train = it.view().second; train.has_released_) {
    msgr_ << "-1\n";
    return;
  }
  auto &train = (*it).second;
  train.has_released_ = true;
  for(stn_num_t i = 0; i < train.stn_num_; ++i)
    stn_hid_train_info_multimap_.insert(
      train.stn_list_[i].hash(), ism::pair<train_hid_t, stn_num_t>(htid, i));
  static TrainSeatStatus train_seat_status;
  train_seat_status.initialize(train.max_seat_num_, train.stn_num_);
  for(days_count_t i = train.start_date_.count(); i <= train.final_date_.count(); ++i)
    train_hid_seats_map_.insert(
      ism::pair<train_hid_t, days_count_t>(htid, i), train_seat_status);
    msgr_ << "0\n";
}

void TrainManager::QueryTrain(const train_id_t &train_id, date_md_t train_departure_date) {
  auto htid = train_id.hash();
  const auto it = train_hid_train_map_.find(htid);
  if(it == train_hid_train_map_.end()) {
    msgr_ << "-1\n";
    return;
  }
  const auto &train = (*it).second;
  if(!train.check_train_departure_date(train_departure_date)) {
    msgr_ << "-1\n";
    return;
  }

  static seat_num_list_t seat_num_list;
  if(train.has_released_) {
    auto it2 = train_hid_seats_map_.find(
      ism::pair<train_hid_t, days_count_t>(htid, train_departure_date.count()));
    seat_num_list = (*it2).second.seat_num_list_;
  } else {
    for(size_t i = 0; i < train.stn_num_; ++i)
      seat_num_list[i] = train.max_seat_num_;
  }

  // train info
  msgr_ << train.train_id_ << ' ' << train.train_type_ << '\n';
  const date_time_t date_time(train_departure_date, train.start_time_);
  for(size_t i = 0; i < train.stn_num_; ++i) {
    msgr_ << train.stn_list_[i] << ' ';
    if(i == 0)
      msgr_ << date_time_t::default_c_str();
    else {
      msgr_ << date_time_t(date_time + train.arrival_time_list_[i]).string();
    }
    msgr_ << " -> ";
    if(i == train.stn_num_ - 1)
      msgr_ << date_time_t::default_c_str();
    else {
      msgr_ << date_time_t(date_time + train.departure_time_list_[i]).string();
    }
    msgr_ << ' ' << train.accumulative_price_list_[i] << ' ';
    if(i == train.stn_num_ - 1)
      msgr_ << 'x' << '\n';
    else
      msgr_ << seat_num_list[i] << '\n';
  }
}

void TrainManager::QueryTicket(
  stn_name_t from_stn, stn_name_t dest_stn,
  date_md_t passenger_departure_date, bool is_cost_order) {

  auto from_hsid = from_stn.hash();
  auto dest_hsid = dest_stn.hash();

  // already sorted in terms of htid.
  const auto from_train_list = stn_hid_train_info_multimap_.search(from_hsid);
  const auto dest_train_list = stn_hid_train_info_multimap_.search(dest_hsid);

  // It's said that this goes against compilation... But I haven't seen it yet.
  struct QueryResultType {
    std::string msg;
    time_dur_t  time;
    price_t     cost;
    train_id_t  train_id;

    QueryResultType() = default;
    QueryResultType(std::string _msg, time_dur_t _time, price_t _cost, train_id_t _train_id)
      : msg(std::move(_msg)), time(_time), cost(_cost), train_id(_train_id) {}
  };

  ism::vector<QueryResultType> ret_vec;

  for(size_t from_ptr = 0, dest_ptr = 0;
      from_ptr < from_train_list.size() && dest_ptr < dest_train_list.size(); ) {
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
    auto seats_it = train_hid_seats_map_.find(
      ism::pair<train_hid_t, days_count_t>(train.hash(), train_departure_date.count()));
    auto &train_seat_status = (*seats_it).second;
    auto available_seat_num = train_seat_status.available_seat_num(from_ord, dest_ord);
    ism::Messenger tmp_msgr;
    tmp_msgr << train.train_id_ << ' ' << from_stn << ' '
             << date_time_t(train_departure_date_time + train.departure_time_list_[from_ord]).string()
             << " -> " << dest_stn << ' '
             << date_time_t(train_departure_date_time + train.arrival_time_list_[dest_ord]).string()
             << ' ' << cost << ' ' << available_seat_num << '\n';
    ret_vec.emplace_back(tmp_msgr.str(), time, cost, train.train_id_);
    ++from_ptr; ++dest_ptr;
  }

  if(is_cost_order)
    ism::sort(
      ret_vec.begin(), ret_vec.end(),
      [](const QueryResultType &A, const QueryResultType &B) {
        if(A.cost != B.cost) return A.cost < B.cost;
        return A.train_id < B.train_id;
      });
  else
    ism::sort(
      ret_vec.begin(), ret_vec.end(),
      [](const QueryResultType &A, const QueryResultType &B) {
        if(A.time != B.time) return A.time < B.time;
        return A.train_id < B.train_id;
      });

  msgr_ << ret_vec.size() << '\n';
  for(const auto& ret : ret_vec)
    msgr_ << ret.msg;
}

void TrainManager::QueryTransfer(
  stn_name_t from_stn, stn_name_t dest_stn,
  date_md_t passenger_departure_date, bool is_cost_order) {
  auto from_hsid = from_stn.hash();
  auto dest_hsid = dest_stn.hash();

  // already sorted in terms of htid.
  const auto from_train_list = stn_hid_train_info_multimap_.search(from_hsid);
  const auto dest_train_list = stn_hid_train_info_multimap_.search(dest_hsid);

  bool success = false;
  price_t    cost;
  time_dur_t time;

  struct ResultInfoType {
    TrainType from_train, dest_train;
    date_time_t date_time_SS, date_time_ST, date_time_TS, date_time_TT;
    date_md_t from_train_dep_date, dest_train_dep_date;
    stn_num_t stn_ord_SS, stn_ord_ST, stn_ord_TS, stn_ord_TT;
    hash_stn_name_t interval_stn_hid;

    ResultInfoType() = default;
    /*
    ResultInfoType(
      const TrainType &_from_train, const TrainType &_dest_train,
      date_time_t _date_time_SS, date_time_t _date_time_ST,
      date_time_t _date_time_TS, date_time_t _date_time_TT,
      const stn_name_t &_interval_stn)
    : from_train(_from_train), dest_train(_dest_train),
      date_time_SS(_date_time_SS), date_time_ST(_date_time_ST),
      date_time_TS(_date_time_TS), date_time_TT(_date_time_TT),
      interval_stn(_interval_stn) {}
    */
  } result_info;

  struct InfoType {
    train_hid_t train_hid;
    stn_num_t self_ord, other_ord;
    hash_stn_name_t interval_stn_hid;
    InfoType() = default;
    InfoType(train_hid_t _train_hid, stn_num_t _self_ord, stn_num_t _other_ord, hash_stn_name_t _other_stn_hid)
      : train_hid(_train_hid), self_ord(_self_ord), other_ord(_other_ord), interval_stn_hid(_other_stn_hid) {}
  };

  ism::vector<InfoType> from_info_list;
  ism::vector<InfoType> dest_info_list;

  for(const auto &[train_hid, stn_ord] : from_train_list) {
    const auto it = train_hid_train_map_.find(train_hid);
    const auto &train = (*it).second;
    for(stn_num_t i = stn_ord + 1; i < train.stn_num_; ++i)
      from_info_list.emplace_back(train_hid, stn_ord, i, train.stn_list_[i].hash());
  }
  for(const auto &[train_hid, stn_ord] : dest_train_list) {
    const auto it = train_hid_train_map_.find(train_hid);
    const auto &train = (*it).second;
    for(stn_num_t i = 0; i < stn_ord; ++i)
      dest_info_list.emplace_back(train_hid, stn_ord, i, train.stn_list_[i].hash());
  }

  ism::sort(from_info_list.begin(), from_info_list.end(),
    [] (const InfoType &A, const InfoType &B) {
    return A.interval_stn_hid < B.interval_stn_hid;
  });
  ism::sort(dest_info_list.begin(), dest_info_list.end(),
    [] (const InfoType &A, const InfoType &B) {
    return A.interval_stn_hid < B.interval_stn_hid;
  });

  for(size_t from_l = 0, dest_l = 0, from_r = 0, dest_r = 0;
    from_l < from_info_list.size() && dest_l < dest_info_list.size(); ) {
    if(from_info_list[from_l].interval_stn_hid < dest_info_list[dest_l].interval_stn_hid) { ++from_l; continue; }
    if(from_info_list[from_l].interval_stn_hid > dest_info_list[dest_l].interval_stn_hid) { ++dest_l; continue; }
    from_r = from_l; dest_r = dest_l;
    auto interval_stn_hid = dest_info_list[dest_l].interval_stn_hid;
    while(from_r + 1 < from_info_list.size() &&
          from_info_list[from_r + 1].interval_stn_hid == interval_stn_hid) ++from_r;
    while(dest_r + 1 < dest_info_list.size() &&
          dest_info_list[dest_r + 1].interval_stn_hid == interval_stn_hid) ++dest_r;

    ism::vector<TrainType> train_list_S;
    ism::vector<TrainType> train_list_T;
    train_list_S.reserve(from_r - from_l + 1);
    train_list_T.reserve(dest_r - dest_l + 1);
    for(size_t i = from_l; i <= from_r; ++i)
      train_list_S.push_back(train_hid_train_map_.find(from_info_list[i].train_hid).view().second);
    for(size_t j = dest_l; j <= dest_r; ++j)
      train_list_T.push_back(train_hid_train_map_.find(dest_info_list[j].train_hid).view().second);

    for(size_t i = from_l; i <= from_r; ++i)
      for(size_t j = dest_l; j <= dest_r; ++j) {
        auto &from_train = train_list_S[i - from_l], &dest_train = train_list_T[j - dest_l];
        auto stn_ord_SS = from_info_list[i].self_ord, stn_ord_ST = from_info_list[i].other_ord;
        auto stn_ord_TT = dest_info_list[j].self_ord, stn_ord_TS = dest_info_list[j].other_ord;

        if(from_train.hash() == dest_train.hash()) continue;

        // check date
        auto from_train_dep_date =
          from_train.get_train_departure_date(passenger_departure_date, stn_ord_SS);
        if(!from_train.check_train_departure_date(from_train_dep_date)) continue;
        date_time_t date_time_SS =
          date_time_t(from_train_dep_date, from_train.start_time_) + from_train.departure_time_list_[stn_ord_SS];
        date_time_t date_time_ST =
          date_time_t(from_train_dep_date, from_train.start_time_) + from_train.arrival_time_list_[stn_ord_ST];

        auto passenger_dep_T_date = date_time_ST.date_md();
        if(time_hm_t dest_time_hm_TS = dest_train.start_time_ + dest_train.departure_time_list_[stn_ord_TS];
          date_time_ST.time_hm() > dest_time_hm_TS)
          passenger_dep_T_date += days(1);
        auto dest_train_dep_date =
          dest_train.get_train_departure_date(passenger_dep_T_date, stn_ord_TS);
        if(dest_train.final_date_ < dest_train_dep_date) continue;
        if(dest_train.start_date_ > dest_train_dep_date)
          dest_train_dep_date = dest_train.start_date_; // do not use passenger_dep_T_date again.
        date_time_t date_time_TS =
          date_time_t(dest_train_dep_date, dest_train.start_time_) + dest_train.departure_time_list_[stn_ord_TS];
        date_time_t date_time_TT =
          date_time_t(dest_train_dep_date, dest_train.start_time_) + dest_train.arrival_time_list_[stn_ord_TT];

        auto cost_here = from_train.cost(stn_ord_SS, stn_ord_ST) + dest_train.cost(stn_ord_TS, stn_ord_TT);
        auto time_here = minutes(date_time_TT.count() - date_time_SS.count());

        if(!success || (is_cost_order && [&] {
            if(cost_here != cost) return cost_here < cost;
            if(time_here != time) return time_here < time;
            if(from_train.train_id_ != result_info.from_train.train_id_)
              return from_train.train_id_ < result_info.from_train.train_id_;
            return dest_train.train_id_ < result_info.dest_train.train_id_;
          } ()) || (!is_cost_order && [&] {
          if(time_here != time) return time_here < time;
          if(cost_here != cost) return cost_here < cost;
          if(from_train.train_id_ != result_info.from_train.train_id_)
            return from_train.train_id_ < result_info.from_train.train_id_;
          return dest_train.train_id_ < result_info.dest_train.train_id_;
          } ())) {

          success = true;
          cost = cost_here;
          time = time_here;
          result_info.from_train = from_train;
          result_info.dest_train = dest_train;
          result_info.date_time_SS = date_time_SS;
          result_info.date_time_ST = date_time_ST;
          result_info.date_time_TS = date_time_TS;
          result_info.date_time_TT = date_time_TT;
          result_info.from_train_dep_date = from_train_dep_date;
          result_info.dest_train_dep_date = dest_train_dep_date;
          result_info.interval_stn_hid = interval_stn_hid;
          result_info.stn_ord_SS = stn_ord_SS;
          result_info.stn_ord_ST = stn_ord_ST;
          result_info.stn_ord_TS = stn_ord_TS;
          result_info.stn_ord_TT = stn_ord_TT;
        }
      }
    ++from_l; ++dest_l;
  }

  if(!success) {
    msgr_ << "0\n";
    return;
  }

  const auto seat_num_list_S_it =
    train_hid_seats_map_.find(
      ism::make_pair(result_info.from_train.hash(), result_info.from_train_dep_date.count()));
  auto seat_num_S = (*seat_num_list_S_it).second.available_seat_num(
    result_info.stn_ord_SS, result_info.stn_ord_ST);
  const auto seat_num_list_T_it =
  train_hid_seats_map_.find(
    ism::make_pair(result_info.dest_train.hash(), result_info.dest_train_dep_date.count()));
  auto seat_num_T = (*seat_num_list_T_it).second.available_seat_num(
    result_info.stn_ord_TS, result_info.stn_ord_TT);

  msgr_ << result_info.from_train.train_id_ << ' ' << from_stn << ' '
        << result_info.date_time_SS.string() << " -> "
        << result_info.from_train.stn_list_[result_info.stn_ord_ST] << ' '
        << result_info.date_time_ST.string() << ' '
        << result_info.from_train.cost(result_info.stn_ord_SS, result_info.stn_ord_ST) << ' '
        << seat_num_S << '\n'
        << result_info.dest_train.train_id_ << ' '
        << result_info.dest_train.stn_list_[result_info.stn_ord_TS] << ' '
        << result_info.date_time_TS.string() << " -> "
        << dest_stn << ' '
        << result_info.date_time_TT.string() << ' '
        << result_info.dest_train.cost(result_info.stn_ord_TS, result_info.stn_ord_TT) << ' '
        << seat_num_T << '\n';
}

TicketOrderType TrainManager::BuyTicket(
  const username_t &username,
  const train_id_t &train_id,
  stn_name_t from_stn, stn_name_t dest_stn,
  date_md_t passenger_departure_date, seat_num_t ticket_num,
  bool accept_waitlist) {

  auto thid = train_id.hash();
  const auto train_it = train_hid_train_map_.find(thid);
  const auto &train = (*train_it).second;

  if(ticket_num > train.max_seat_num_) {
    msgr_ << "-1\n";
    return {};
  }

  stn_num_t from_ord = train.stn_num_ + 1;
  stn_num_t dest_ord = train.stn_num_ + 1;
  auto from_stn_hash = from_stn.hash();
  auto dest_stn_hash = dest_stn.hash();
  for(stn_num_t i = 0; i < train.stn_num_; ++i) {
    auto stn_hash = train.stn_list_[i].hash();
    if(stn_hash == from_stn_hash) from_ord = i;
    if(stn_hash == dest_stn_hash) dest_ord = i;
  }
  if(from_ord == train.stn_num_ + 1 || dest_ord == train.stn_num_ + 1 || from_ord >= dest_ord) {
    msgr_ << "-1\n";
    return {};
  }
  auto train_departure_date = train.get_train_departure_date(passenger_departure_date, from_ord);
  if(!train.check_train_departure_date(train_departure_date)) {
    msgr_ << "-1\n";
    return {};
  }
  auto seat_it = train_hid_seats_map_.find(ism::pair<train_hid_t, days_count_t>(thid, train_departure_date.count()));
  auto available_seats = seat_it.view().second.available_seat_num(from_ord, dest_ord);
  if(available_seats < ticket_num) {
    if(accept_waitlist) {
      msgr_ << "queue\n";
      return {
        TicketOrderType::OrderStatus::Pending, username, train_id, from_stn, dest_stn,
        date_time_t(train_departure_date, train.start_time_) + train.departure_time_list_[from_ord],
        date_time_t(train_departure_date, train.start_time_) + train.arrival_time_list_[dest_ord],
        from_ord, dest_ord, train.cost(from_ord, dest_ord), ticket_num, train_departure_date};
    }
    msgr_ << "-1\n";
    return {};
  }
  (*seat_it).second.consume_seat_num(ticket_num, from_ord, dest_ord);
  msgr_ << train.cost(from_ord, dest_ord) * ticket_num << '\n';
  return {
    TicketOrderType::OrderStatus::Success, username, train_id, from_stn, dest_stn,
    date_time_t(train_departure_date, train.start_time_) + train.departure_time_list_[from_ord],
    date_time_t(train_departure_date, train.start_time_) + train.arrival_time_list_[dest_ord],
    from_ord, dest_ord, train.cost(from_ord, dest_ord), ticket_num, train_departure_date};
}

ism::Bplustree<ism::pair<train_hid_t, days_count_t>, TrainSeatStatus>::iterator
TrainManager::get_seat_status_iter(const train_id_t &train_id, date_md_t train_dep_date) {
  return train_hid_seats_map_.find(ism::pair<train_hid_t, days_count_t>(train_id.hash(), train_dep_date.count()));
}

void TrainManager::clean() {
  train_hid_train_map_.clear();
  train_hid_seats_map_.clear();
  stn_hid_train_info_multimap_.clear();
}
}