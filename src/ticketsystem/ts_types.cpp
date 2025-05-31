#include "ts_types.h"

#include "algorithm.h"

namespace ticket_system {

void CmdAddUser::handle(char par_name, const char *beg, size_t n) {
  switch(par_name) {
  case 'c' : cur_username = username_t(beg, n);              break;
  case 'u' : tar_username = username_t(beg, n);              break;
  case 'p' : password     = password_t(beg, n);              break;
  case 'n' : zh_name      = zh_name_t(beg, n);               break;
  case 'm' : mail_addr    = mail_addr_t(beg, n);             break;
  case 'g' : access_lvl   = ism::stoi<access_lvl_t>(beg, n); break;
  default : throw ism::invalid_argument("unknown parameter name.");
  }
}

void CmdLogin::handle(char par_name, const char *beg, size_t n) {
  switch(par_name) {
  case 'u' : username = username_t(beg, n); break;
  case 'p' : password = password_t(beg, n); break;
  default : throw ism::invalid_argument("unknown parameter name.");
  }
}

void CmdLogout::handle(char par_name, const char *beg, size_t n) {
  switch(par_name) {
  case 'u' : username = username_t(beg, n); break;
  default : throw ism::invalid_argument("unknown parameter name.");
  }
}

void CmdQueryProfile::handle(char par_name, const char *beg, size_t n) {
  switch(par_name) {
  case 'c' : cur_username = username_t(beg, n); break;
  case 'u' : tar_username = username_t(beg, n); break;
  default : throw ism::invalid_argument("unknown parameter name.");
  }
}

void CmdModifyProfile::handle(char par_name, const char *beg, size_t n) {
  switch(par_name) {
  case 'c' : cur_username = username_t(beg, n);                                     break;
  case 'u' : tar_username = username_t(beg, n);                                     break;
  case 'p' : password     = password_t(beg, n);              has_password = true;   break;
  case 'n' : zh_name      = zh_name_t(beg, n);               has_zh_name = true;    break;
  case 'm' : mail_addr    = mail_addr_t(beg, n);             has_mail_addr = true;  break;
  case 'g' : access_lvl   = ism::stoi<access_lvl_t>(beg, n); has_access_lvl = true; break;
  default : throw ism::invalid_argument("unknown parameter name.");
  }
}

void CmdAddTrain::handle(char par_name, const char *beg, size_t n) {
  switch(par_name) {
  case 'i' : train_id = train_id_t(beg, n); break;
  case 'n' : stn_num  = ism::stoi<stn_num_t>(beg, n); break;
  case 'm' : seat_num = ism::stoi<seat_num_t>(beg, n); break;
  case 's' : {
    int cnt = 0;
    const char *in_beg = beg, *in_end = beg;
    while(true) {
      ism::advance_until(in_end, '|', ' ');
      stn_list[cnt] = stn_name_t(in_beg, in_end - in_beg);
      ++cnt;
      if(*in_end == ' ') break;
      ism::advance_past(in_end, '|');
      in_beg = in_end;
    }
  } break;
  case 'p' : {
    int cnt = 0;
    const char *in_beg = beg, *in_end = beg;
    while(true) {
      ism::advance_until(in_end, '|', ' ');
      price_list[cnt] = ism::stoi<price_t>(in_beg, in_end - in_beg);
      ++cnt;
      if(*in_end == ' ') break;
      ism::advance_past(in_end, '|');
      in_beg = in_end;
    }
  } break;
  case 'x' : start_time = time_hm_t(beg, n); break;
  case 't' : {
    int cnt = 0;
    const char *in_beg = beg, *in_end = beg;
    while(true) {
      ism::advance_until(in_end, '|', ' ');
      travel_time_list[cnt] = time_dur_t(ism::stoi<time_dur_t::count_t>(in_beg, in_end - in_beg));
      ++cnt;
      if(*in_end == ' ') break;
      ism::advance_past(in_end, '|');
      in_beg = in_end;
    }
  } break;
  case 'o' : {
    int cnt = 0;
    if(*beg == '-') break;
    const char *in_beg = beg, *in_end = beg;
    while(true) {
      ism::advance_until(in_end, '|', ' ');
      stopover_time_list[cnt] = time_dur_t(ism::stoi<time_dur_t::count_t>(in_beg, in_end - in_beg));
      ++cnt;
      if(*in_end == ' ') break;
      ism::advance_past(in_end, '|');
      in_beg = in_end;
    }
  } break;
  case 'd' : {
    const char *in_beg = beg, *in_end = beg;
    ism::advance_until(in_end, '|');
    begin_date = date_md_t(in_beg, in_end - in_beg);
    ism::advance_past(in_end, '|', ' ');
    in_beg = in_end;
    ism::advance_until(in_end, ' ');
    final_date = date_md_t(in_beg, in_end - in_beg);
  } break;
  case 'y' : train_type = static_cast<train_type_t>(*beg); break;
  default : throw ism::invalid_argument("unknown parameter name.");
  }
}

void CmdDeleteTrain::handle(char par_name, const char *beg, size_t n) {
  switch(par_name) {
  case 'i' : train_id = train_id_t(beg, n); break;
  default : throw ism::invalid_argument("unknown parameter name.");
  }
}

void CmdReleaseTrain::handle(char par_name, const char *beg, size_t n) {
  switch(par_name) {
  case 'i' : train_id = train_id_t(beg, n); break;
  default : throw ism::invalid_argument("unknown parameter name.");
  }
}

void CmdQueryTrain::handle(char par_name, const char *beg, size_t n) {
  switch(par_name) {
  case 'i' : train_id       = train_id_t(beg, n); break;
  case 'd' : departure_date = date_md_t(beg, n);  break;
  default : throw ism::invalid_argument("unknown parameter name.");
  }
}

void CmdQueryTicket::handle(char par_name, const char *beg, size_t n) {
  is_cost_order = false;
  switch(par_name) {
  case 's' : departure_stn  = stn_name_t(beg, n); break;
  case 't' : arrival_stn    = stn_name_t(beg, n); break;
  case 'd' : departure_date = date_md_t(beg, n);  break;
  case 'p' : is_cost_order  = (strncmp(beg, "cost", n) == 0);
  default : throw ism::invalid_argument("unknown parameter name.");
  }
}

void CmdQueryTransfer::handle(char par_name, const char *beg, size_t n) {
  is_cost_order = false;
  switch(par_name) {
  case 's' : departure_stn  = stn_name_t(beg, n); break;
  case 't' : arrival_stn    = stn_name_t(beg, n); break;
  case 'd' : departure_date = date_md_t(beg, n);  break;
  case 'p' : is_cost_order  = (strncmp(beg, "cost", n) == 0);
  default : throw ism::invalid_argument("unknown parameter name.");
  }
}

void CmdBuyTicket::handle(char par_name, const char *beg, size_t n) {
  accept_waitlist = false;
  switch(par_name) {
  case 'u' : username        = username_t(beg, n);           break;
  case 'i' : train_id        = train_id_t(beg, n);           break;
  case 'd' : departure_date  = date_md_t(beg, n);            break;
  case 'f' : departure_stn   = stn_name_t(beg, n);           break;
  case 't' : arrival_stn     = stn_name_t(beg, n);           break;
  case 'n' : ticket_num      = ism::stoi<stn_num_t>(beg, n); break;
  case 'q' : accept_waitlist = (strncmp(beg, "true", n) == 0);
  default : throw ism::invalid_argument("unknown parameter name.");
  }
}

void CmdQueryOrder::handle(char par_name, const char *beg, size_t n) {
  switch(par_name) {
  case 'u' : username = username_t(beg, n); break;
  default : throw ism::invalid_argument("unknown parameter name.");
  }
}

void CmdRefundTicket::handle(char par_name, const char *beg, size_t n) {
  order_rank = 1;
  switch(par_name) {
  case 'u' : username   = username_t(beg, n);            break;
  case 'n' : order_rank = ism::stoi<order_id_t>(beg, n); break;
  default : throw ism::invalid_argument("unknown parameter name.");
  }
}

}