#include <daycoinissue.hpp>

ACTION daycoinissue::issue(std::string message_text) {

  eosio::print("daycoinissue::issue was called");

  //require_auth( name("daycoinclaim") );

  // subtract the amount that will go to the WPS
  // subtract the amount for the claimant
  // divide the remainder by the number of accounts that exist in the accounts table
  // add any non-divisable remainder to the WPS amount
  // do all the new issues to each of the appropriate accounts

  uint64_t currentSecondSinceEpoch = current_time_point().sec_since_epoch();
  uint64_t currentMinutesSinceEpoch = currentSecondSinceEpoch/60;
  uint64_t currentHoursSinceEpoch = currentMinutesSinceEpoch/60;
  uint64_t currentDaysSinceEpoch = currentHoursSinceEpoch/24;
  string message = "It has been ";
  message += to_string(currentSecondSinceEpoch);
  message += "; ";
  message += to_string(currentMinutesSinceEpoch);
  message += "; ";
  message += to_string(currentHoursSinceEpoch);
  message += "; ";
  message += to_string(currentDaysSinceEpoch);
  message += "; ";
  message += " sec/min/hrs/days since epoch. ";
  message += message_text;
  name user = name("daycoinclaim");
  daycoinissue::hi(user, message);
}

ACTION daycoinissue::hi(name from, string message) {
  require_auth(from);

  // Init the _message table
  messages_table _messages(get_self(), get_self().value);

  // Find the record from _messages table
  auto msg_itr = _messages.find(from.value);
  if (msg_itr == _messages.end()) {
    // Create a message record if it does not exist
    _messages.emplace(from, [&](auto& msg) {
      msg.user = from;
      msg.text = message;
    });
  } else {
    // Modify a message record if it exists
    _messages.modify(msg_itr, from, [&](auto& msg) {
      msg.text = message;
    });
  }
}

ACTION daycoinissue::clear() {
  require_auth(get_self());

  messages_table _messages(get_self(), get_self().value);

  // Delete all records in _messages table
  auto msg_itr = _messages.begin();
  while (msg_itr != _messages.end()) {
    msg_itr = _messages.erase(msg_itr);
  }
}

EOSIO_DISPATCH(daycoinissue, (hi)(clear)(issue))
