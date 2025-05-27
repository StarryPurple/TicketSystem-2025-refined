#include "ticketsystem.h"

namespace ticket_system {

void TicketSystem::get_command_type(const char *str) {
  switch(str[0]) {
  case 'a' : {
    if(str[4] == 'u') type = CommandType::ADD_USER;
    else type = CommandType::ADD_TRAIN;
  } break;
  case 'l' : {
    if(str[3] == 'i') type = CommandType::LOGIN;
    else type = CommandType::LOGOUT;
  } break;
  case 'r' : {
    if(str[2] == 'l') type = CommandType::RELEASE_TRAIN;
    else type = CommandType::REFUND_TICKET;
  } break;
  case 'q' : {
    switch(str[9]) {
    case 'f' : type = CommandType::QUERY_PROFILE; break;
    case 'i' : type = CommandType::QUERY_TRAIN; break;
    case 'k' : type = CommandType::QUERY_TICKET; break;
    case 'n' : type = CommandType::QUERY_TRANSFER; break;
    case 'c' : type = CommandType::REFUND_TICKET; break;
    default : throw std::invalid_argument("");
    }
  } break;
  case 'm' : type = CommandType::MODIFY_PROFILE; break;
  case 'd' : type = CommandType::DELETE_TRAIN; break;
  case 'b' : type = CommandType::BUY_TICKET; break;
  case 'c' : type = CommandType::CLEAN; break;
  case 'e' : type = CommandType::EXIT; break;
  default : throw std::invalid_argument(""); //...
  }
}


}