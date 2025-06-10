#ifndef TICKETSYSTEM_TICKET_ORDER_MANAGER_H
#define TICKETSYSTEM_TICKET_ORDER_MANAGER_H

#include "bplustree.h"
#include "multi_bplustree.h"
#include "ts_types.h"
#include "messenger.h"
namespace ticket_system {

class TicketOrderManager {

public:

  TicketOrderManager(std::filesystem::path path, ism::Messenger &msgr);
  ~TicketOrderManager() = default;

  void record_buy_ticket(TicketOrderType &ticket_order);
  void QueryOrder(const username_t &username);

  order_id_t find_order_id(const username_t &username, order_id_t order_rank);
  ism::CompressedBufferPool<TicketOrderType>::Visitor
  get_order_visitor(order_id_t order_id) {
    return order_list_.visitor(order_id);
  }
  TicketOrderType get_order(order_id_t order_id) {
    auto visitor = order_list_.visitor(order_id);
    return *visitor.as();
  }
  ism::vector<order_id_t>
  get_ticket_related_orders(const train_id_t &train_id, date_md_t train_dep_date) {
    return train_hid_order_id_map_.search(
      ism::make_pair(train_id.hash(), train_dep_date.count()));
  }
  order_id_t new_order_id() { return order_list_.alloc(); } // private?
  void clean();

private:

  ism::CompressedBufferPool<TicketOrderType> order_list_;
  ism::MultiBplustree<
    user_hid_t, order_id_t, std::less<>, std::greater<>>
  user_hid_order_id_map_; // for query: the latest orders come first.
  ism::MultiBplustree<
    ism::pair<train_hid_t, days_count_t>, order_id_t>
  train_hid_order_id_map_; // for refund: the earliest orders come first.
  ism::Messenger &msgr_;
};

}

#endif