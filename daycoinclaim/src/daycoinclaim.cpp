//#include <daycoinclaim.hpp>

#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>

#include <claimant.hpp>
#include <claim.hpp>

#include <daycoinissue.cpp>
#include <daycoinaccount.hpp>

using namespace std;
using namespace eosio;


class [[eosio::contract("daycoinclaim")]] daycoinclaim : public contract {
  
  public:
  
    using contract::contract;

    [[eosio::action]]
    void makeclaim(eosio::name account_name) {
      
      //eosio::print("daycoinclaim::makeclaim called by ", get_self().to_string());

      // make sure the account_name is signed by the user calling the action
      require_auth(account_name);
      //require_auth(get_first_receiver());

      bool has_day_coin_account = determine_day_coin_account(account_name, get_self());
      //eosio::print("has_day_coin_account: ", has_day_coin_account);
      // determine if this user has a daycoin account
      if (!has_day_coin_account) {
        // if not create a daycoinissue.account
        eosio::print("createacct should be called after this line");
        create_account(account_name);
      }

      // determine if this user has already registered as a claimant on claim_day + 1
      //   if not then create claimants record
      // determine if we are >= the seconds_until_next_claim
      //   if true then determine how many timespans(days) we are past the seconds_until_next_claim
      // for each timespan(day)
      //   daycoinissue.issue(claim_day, winner, claimants)
      //   create claims table record
      //   clear claimants
      //   register winner as first claimant for new day

    };

  private:

    uint32_t time_between_claims_in_seconds = 86400; // 86400 = 24 hours

    void create_account(name account_name) {
      daycoinissue::createacct_action createacct_act { "daycoinissue"_n, { get_self(), "active"_n } };
      createacct_act.send(account_name);
    };

    uint64_t getclaimday() {
      return 1;
    };

    void clearclaimants() {
    };

    bool determine_day_coin_account(name account_name, name scope) {
      accounts_table accounts("daycoinissue"_n, scope.value);
      auto iterator = accounts.find(account_name.value);
      bool has_account = iterator != accounts.end();
      eosio::print("has_account: ", has_account);
      return has_account;
    };

    struct [[eosio::table]] messages {
      name    user;
      string  text;
      auto primary_key() const { return user.value; }
    };
    typedef multi_index<name("messages"), messages> messages_table;
};

//EOSIO_DISPATCH( daycoinclaim, (makeclaim) )
