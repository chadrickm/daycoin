#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <string>

#include <daycoinaccount.hpp>

using namespace std;
using namespace eosio;

CONTRACT daycoinissue : public contract {
  public:
    using contract::contract;

    ACTION hi(name from, string message);
    ACTION clear();

    ACTION createacct(name account_name);
    ACTION issue(std::string message_text);

    using createacct_action = action_wrapper<"createacct"_n, &daycoinissue::createacct>;
    using issue_action = action_wrapper<"issue"_n, &daycoinissue::issue>;

  private:
    TABLE messages {
      name    user;
      string  text;
      auto primary_key() const { return user.value; }
    };
    typedef multi_index<name("messages"), messages> messages_table;
};
