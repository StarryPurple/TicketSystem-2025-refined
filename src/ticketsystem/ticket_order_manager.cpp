#include "ticket_order_manager.h"

namespace ticket_system {

static constexpr int BUF_CAPA = 100, K_DIST = 2;

TicketOrderManager::TicketOrderManager(std::filesystem::path path, ism::Messenger &msgr)
: order_list_(path.string() + "-olist", BUF_CAPA, K_DIST),
  user_hid_order_id_map_(path.string() + "-huid_order", BUF_CAPA, K_DIST),
  train_hid_order_id_map_(path.string() + "-htid_order", BUF_CAPA, K_DIST),
  msgr_(msgr) {}

void TicketOrderManager::record_buy_ticket(TicketOrderType &ticket_order) {
  ticket_order.order_id_ = new_order_id();
  auto visitor = order_list_.visitor(ticket_order.order_id_);
  auto *order_ptr = visitor.as_mut();
  *order_ptr = ticket_order;
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
    msgr_ << order_list_.visitor(order_id).as()->string() << '\n';
  }
}

order_id_t TicketOrderManager::find_order_id(const username_t &username, order_id_t order_rank) {
  auto order_list = user_hid_order_id_map_.search(username.hash());
  if(order_list.size() < order_rank)
    return INVALID_ORDER_ID;
  return order_list[order_rank - 1];
}

void TicketOrderManager::clean() {
  order_list_.clear();
  user_hid_order_id_map_.clear();
  train_hid_order_id_map_.clear();
}

}