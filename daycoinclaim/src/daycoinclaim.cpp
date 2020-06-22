#include <daycoinclaim.hpp>

uint32_t time_between_claims_in_seconds = 86400; // 86400 = 24 hours

ACTION daycoinclaim::makeclaim(eosio::name account_name) {
  
  eosio::print("daycoinclaim::makeclaim called");

  require_auth(account_name);

  {
    daycoinissue::issue_action issue_act { "daycoinissue"_n, { get_self(), "active"_n } };
    issue_act.send("calling from makeclaim");
  }

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
  
  // determine if this user has a daycoin account
  //   if not create a daycoinissue.account
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

uint64_t daycoinclaim::getclaimday() {
  return 1;
};

void daycoinclaim::clearclaimants() {
};

EOSIO_DISPATCH( daycoinclaim, (makeclaim) )
