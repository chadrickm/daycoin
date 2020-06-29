/*#include <eosio/eosio.hpp>
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
    void hi(name from, string message);
    [[eosio::action]]
    void clear();

    [[eosio::action]]
    void createacct(name account_name);
    [[eosio::action]]
    void issue(std::string message_text);

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
*/