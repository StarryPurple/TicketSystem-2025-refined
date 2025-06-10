#include "ticket_order_manager.h"

namespace ticket_system {

static constexpr int BUF_CAPA = 256, K_DIST = 2;

TicketOrderManager::TicketOrderManager(std::filesystem::path path, ism::Messenger &msgr)
: index_pool_(path.string() + "-order_id"),
  user_hid_order_map_(path.string() + "-huid_order", BUF_CAPA, K_DIST),
  train_hid_order_map_(path.string() + "-htid_order", BUF_CAPA, K_DIST),
  msgr_(msgr) {}

void TicketOrderManager::record_buy_ticket(TicketOrderType &ticket_order) {
  ticket_order.order_id_ = new_order_id();
  user_hid_order_map_.insert(ticket_order.username_.hash(), ticket_order);
  train_hid_order_map_.insert(
    ism::make_pair(ticket_order.train_id().hash(), ticket_order.train_dep_date().count()),
    ticket_order);
}

void TicketOrderManager::QueryOrder(const username_t &username) {
  auto order_list = user_hid_order_map_.search(username.hash());
  msgr_ << order_list.size() << '\n';
  for(auto &order : order_list) {
    msgr_ << order << '\n';
  }
}

insomnia::MultiBplustree<unsigned long, TicketOrderType, std::less<>, std::greater<>>::iterator
TicketOrderManager::find_order_iter(const username_t &username, order_id_t order_rank) {
  auto huid = username.hash();
  order_id_t rank = 0;
  auto it = user_hid_order_map_.find_upper(huid);
  while(it != user_hid_order_map_.end() && it.view().first == huid) {
    if(++rank == order_rank)
      return it;
    ++it;
  }
  return huid_order_multbpt::iterator();
}

void TicketOrderManager::update_user_order_stat(const TicketOrderType &ticket_order) {
  // ticket orders with same order_id are recognized identical.
  auto it = user_hid_order_map_.find(ticket_order.username_.hash(), ticket_order);
  (*it).second.status_ = ticket_order.status_;
}

void TicketOrderManager::update_train_order_stat(const TicketOrderType &ticket_order) {
  // ticket orders with same order_id are recognized identical.
  auto it = train_hid_order_map_.find(
    ism::make_pair(ticket_order.train_id().hash(), ticket_order.train_dep_date().count()),
    ticket_order);
  (*it).second.status_ = ticket_order.status_;
}

void TicketOrderManager::recover_ticket(const TicketOrderType &refunded_ticket_order, TrainSeatStatus &seat_status) {
  auto key = ism::make_pair(
      refunded_ticket_order.train_id().hash(),
      refunded_ticket_order.train_dep_date().count());
  for(auto it = train_hid_order_map_.find_upper(key);
      it != train_hid_order_map_.end() && it.view().first == key; ++it) {
    auto &train_order_view = it.view().second;
    if(!train_order_view.is_pending()) continue;
    if(train_order_view.ticket_num() >
       seat_status.available_seat_num(train_order_view.from_stn_ord(), train_order_view.dest_stn_ord()))
      continue;
    auto &train_order = (*it).second;
    train_order.set_success();
    seat_status.consume_seat_num(train_order.ticket_num(), train_order.from_stn_ord(), train_order.dest_stn_ord());

    update_user_order_stat(train_order);
  }
}


void TicketOrderManager::clean() {
  index_pool_.clear();
  user_hid_order_map_.clear();
  train_hid_order_map_.clear();
}

}