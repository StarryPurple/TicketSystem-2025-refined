#include "ticket_order_manager.h"

namespace ticket_system {

static constexpr int BUF_CAPA = 96, K_DIST = 2;

TicketOrderManager::TicketOrderManager(std::filesystem::path path, ism::Messenger &msgr)
: order_id_order_map_(path.string() + "-otime", BUF_CAPA, K_DIST),
  user_hid_order_id_map_(path.string() + "-huid_order", BUF_CAPA, K_DIST),
  train_hid_order_id_map_(path.string() + "-htid_order", BUF_CAPA, K_DIST),
  order_id_allocator(path.string() + "-oid_alloc"),
  msgr_(msgr) {}

void TicketOrderManager::record_buy_ticket(TicketOrderType &ticket_order) {
  ticket_order.order_id_ = new_order_id();
  order_id_order_map_.insert(ticket_order.order_id_, ticket_order);
  user_hid_order_id_map_.insert(ticket_order.username_.hash(), ticket_order.order_id_);
  train_hid_order_id_map_.insert(
    ism::make_pair(ticket_order.train_id_.hash(), ticket_order.train_dep_date_.count()),
    ticket_order.order_id_);
}

void TicketOrderManager::QueryOrder(const username_t &username) {
  // ... Yes, it's a bit too slow.
  auto order_list = user_hid_order_id_map_.search(username.hash());
  msgr_ << order_list.size() << '\n';
  for(auto &order_id : order_list) {
    msgr_ << order_id_order_map_.search(order_id)->string() << '\n';
  }
}

order_id_t TicketOrderManager::find_order_id(const username_t &username, order_id_t order_rank) {
  auto order_list = user_hid_order_id_map_.search(username.hash());
  if(order_list.size() < order_rank)
    return INVALID_ORDER_ID;
  return order_list[order_rank - 1];
}

ism::vector<order_id_t>
TicketOrderManager::get_ticket_related_orders(const train_id_t &train_id, date_md_t train_dep_date) {
  return train_hid_order_id_map_.search(ism::make_pair(train_id.hash(), train_dep_date.count()));
}

TicketOrderType TicketOrderManager::get_order(order_id_t order_id) {
  return *order_id_order_map_.search(order_id);
}

ism::Bplustree<order_id_t, TicketOrderType>::iterator
TicketOrderManager::get_order_iter(order_id_t order_id) {
  return order_id_order_map_.find(order_id);
}

void TicketOrderManager::clean() {
  user_hid_order_id_map_.clear();
  train_hid_order_id_map_.clear();
  order_id_allocator.clear();
}

}