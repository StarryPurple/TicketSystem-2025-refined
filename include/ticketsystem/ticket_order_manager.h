#ifndef TICKETSYSTEM_TICKET_ORDER_MANAGER_H
#define TICKETSYSTEM_TICKET_ORDER_MANAGER_H

#include "bplustree.h"
#include "multi_bplustree.h"
#include "ts_types.h"
#include "messenger.h"
namespace ticket_system {

class TicketOrderManager {

  using huid_order_multbpt = ism::MultiBplustree<
    user_hid_t, TicketOrderType, std::less<>, std::greater<>>;
  using htid_order_multbpt = ism::MultiBplustree<
    ism::pair<train_hid_t, days_count_t>, TicketOrderType>;

public:

  TicketOrderManager(std::filesystem::path path, ism::Messenger &msgr);
  ~TicketOrderManager() = default;

  // order allocated here.
  void record_buy_ticket(TicketOrderType &ticket_order);
  void QueryOrder(const username_t &username);

  huid_order_multbpt::iterator
  find_order_iter(const username_t &username, order_id_t order_rank);
  ism::vector<TicketOrderType>
  get_ticket_related_orders(const train_id_t &train_id, date_md_t train_dep_date) {
    return train_hid_order_map_.search(
      ism::make_pair(train_id.hash(), train_dep_date.count()));
  }
  void update_user_order_stat(const TicketOrderType &ticket_order);
  void update_train_order_stat(const TicketOrderType &ticket_order);
  void recover_ticket(const TicketOrderType &refunded_ticket_order, TrainSeatStatus &seat_status);
  order_id_t new_order_id() { return index_pool_.alloc(); } // private?
  void clean();

private:

  // ism::CompressedBufferPool<TicketOrderType> order_list_;
  ism::IndexPool index_pool_;
  huid_order_multbpt user_hid_order_map_; // for query: the latest orders come first.
  htid_order_multbpt train_hid_order_map_; // for refund: the earliest orders come first.
  ism::Messenger &msgr_;
};

}

#endif