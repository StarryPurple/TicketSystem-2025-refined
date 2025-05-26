#ifndef TICKETSYSTEM_TS_TYPES_H
#define TICKETSYSTEM_TS_TYPES_H

#include "array.h"

namespace ticket_system {

namespace ism = insomnia;

template <size_t N>
using en_str_t = ism::array<char, N>;
template <size_t N>
using zh_str_t = ism::array<char, N * 4>;

using username_t   = en_str_t<20>;
using pwd_t        = en_str_t<30>;
using zh_name_t    = zh_str_t<5>;
using mail_t       = en_str_t<30>;
using access_lvl_t = short;

using train_id_t   = en_str_t<20>;
using stn_num_t    = short;
using stn_name_t   = zh_str_t<10>;
using seat_num_t   = int;
using price_t      = int;
using train_type_t = char;

// frequency: SF ~ 1e6, F ~ 1e5, N ~ 1e4, R ~ 1e2
enum class CommandType {
  Invalid,
  ADD_USER,       // N
  LOGIN,          // F
  LOGOUT,         // F
  QUERY_PROFILE,  // SF
  MODIFY_PROFILE, // F
  ADD_TRAIN,      // N
  DELETE_TRAIN,   // N
  RELEASE_TRAIN,  // N
  QUERY_TRAIN,    // N
  QUERY_TICKET,   // SF
  QUERY_TRANSFER, // N
  BUY_TICKET,     // SF
  QUERY_ORDER,    // F
  REFUND_TICKET,  // N
  CLEAN,          // R
  EXIT            // R
};

}


#endif