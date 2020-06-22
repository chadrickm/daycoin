#include <daycoinclaim.hpp>

uint32_t time_between_claims_in_seconds = 86400; // 86400 = 24 hours

ACTION daycoinclaim::makeclaim(eosio::name account_name) {
  
  eosio::print("daycoinclaim::makeclaim called");

  // make sure the account_name is signed by the user calling the action
  require_auth(account_name);

  bool has_day_coin_account = daycoinclaim::determine_day_coin_account(account_name);
  // determine if this user has a daycoin account
  if (!has_day_coin_account) {
    // if not create a daycoinissue.account
    eosio::print("createacct should be called after this line");
    {
      daycoinissue::createacct_action createacct_act { "daycoinissue"_n, { get_self(), "active"_n } };
      createacct_act.send(account_name);
    }
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

/*
  messages_table messages("daycoinissue"_n, "daycoinissue"_n.value);
  auto iterator = messages.find(account_name.value);
  if (iterator == messages.end()) {
    {
      daycoinissue::issue_action issue_act { "daycoinissue"_n, { get_self(), "active"_n } };
      issue_act.send(">> record was NOT found");
    }
  }
  else {
    {
      daycoinissue::issue_action issue_act { "daycoinissue"_n, { get_self(), "active"_n } };
      issue_act.send(">> record was found");
    }
  }
*/

uint64_t daycoinclaim::getclaimday() {
  return 1;
};

void daycoinclaim::clearclaimants() {
};

bool daycoinclaim::determine_day_coin_account(name account_name) {
  accounts_table accounts("daycoinissue"_n, "daycoinissue"_n.value);
  auto iterator = accounts.find(account_name.value);
  bool has_account = iterator != accounts.end();
  eosio::print("has_account: ", has_account);
  return has_account;
};

EOSIO_DISPATCH( daycoinclaim, (makeclaim) )
