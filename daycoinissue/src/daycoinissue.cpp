//#include <daycoinissue.hpp>

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <string>

#include <daycoinaccount.hpp>

using namespace std;
using namespace eosio;

class [[eosio::contract("daycoinissue")]] daycoinissue : public contract {
  public:
    using contract::contract;

    [[eosio::action]]
    void createacct(eosio::name account_name) {

      // accounts will only be created through the daycoinclaim account
      //require_auth( name("daycoinclaim") );

      accounts_table accounts(get_self(), get_self().value);
      account_t new_entry = account_t(account_name, current_time_point(), false, "");
      accounts.emplace(account_name, [&](auto & entry) {
        entry = new_entry;
      });
    };

    [[eosio::action]]
    void issue(std::string message_text) {

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

    [[eosio::action]]
    void hi(name from, string message) {
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

    [[eosio::action]]
    void clear() {
      require_auth(get_self());

      accounts_table _accounts(get_self(), get_self().value);

      // Delete all records in _accounts table
      auto iterator = _accounts.begin();
      while (iterator != _accounts.end()) {
        iterator = _accounts.erase(iterator);
      }
    }

    using createacct_action = action_wrapper<"createacct"_n, &daycoinissue::createacct>;
    using issue_action = action_wrapper<"issue"_n, &daycoinissue::issue>;
  
  private:
    struct [[eosio::table]] messages {
      name    user;
      string  text;
      auto primary_key() const { return user.value; }
    };
    typedef multi_index<name("messages"), messages> messages_table;
};

//EOSIO_DISPATCH(daycoinissue, (hi)(clear)(issue)(createacct))
