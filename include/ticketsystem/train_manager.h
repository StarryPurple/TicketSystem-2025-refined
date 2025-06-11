#ifndef TICKETSYSTEM_TRAIN_MANAGER_H
#define TICKETSYSTEM_TRAIN_MANAGER_H

#include "bplustree.h"
#include "ts_types.h"
#include "messenger.h"

namespace ticket_system {

class TrainManager {
public:
  TrainManager(std::filesystem::path path, ism::Messenger &msgr);
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
  ism::Bplustree<ism::pair<days_count_t, train_hid_t>, TrainSeatStatus>::iterator
  get_seat_status_iter(const train_id_t &train_id, date_md_t train_dep_date);
  void clean();

private:

  ism::Bplustree<train_hid_t, TrainType> train_hid_train_map_;
  ism::Bplustree<ism::pair<days_count_t, train_hid_t>, TrainSeatStatus> train_hid_seats_map_;
  // stores trains that pass this station in the form of [htid, #the ordinal of the station of the train]
  // only to be enlarged in ReleaseTrain.
  // So via this method, only released trains can be seen.
  ism::MultiBplustree<hash_stn_name_t, ism::pair<train_hid_t, stn_num_t>> stn_hid_train_info_multimap_;
  ism::Messenger &msgr_;
};

}

#endif