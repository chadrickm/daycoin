#ifndef DAYCOINCLAIM
#define DAYCOINCLAIM

#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>

#include <claimant.hpp>
#include <claim.hpp>

#include <daycoinissue.hpp>
#include <daycoinaccount.hpp>

using namespace std;
using namespace eosio;


CONTRACT daycoinclaim : public contract {
  
  public:
  
    using contract::contract;

    ACTION makeclaim(name account_name);
 
  private:

    uint32_t time_between_claims_in_seconds;

    void clearclaimants();
    uint64_t getclaimday();
    void issue_test();
    bool determine_day_coin_account(name account_name);

    TABLE messages {
      name    user;
      string  text;
      auto primary_key() const { return user.value; }
    };
    typedef multi_index<name("messages"), messages> messages_table;
};

#endif