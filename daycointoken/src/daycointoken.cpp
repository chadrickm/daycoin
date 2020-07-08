#include <daycointoken.hpp>

ACTION daycointoken::registeracct(const name& account_name, const string& voice_account_name)
{
   require_auth( account_name );

   day_accounts_table accounts( get_self(), get_self().value );
   auto iterator = accounts.find( account_name.value );
   check( iterator == accounts.end(), "Day account already exists." );

   accounts.emplace( get_self(), [&]( auto& account ) {
      account.account_name = account_name;
      account.voice_account_name = voice_account_name;
      account.voice_post_hash = "test";
      account.is_synced = false;
   });
}

ACTION daycointoken::clearallacct()
{
   require_auth( name("daycoinadmin") );

   day_accounts_table accounts( get_self(), get_self().value );
   auto iterator = accounts.begin( );
   while (iterator != accounts.end()) {
      iterator = accounts.erase(iterator);
   }
}

ACTION daycointoken::valvoiceacct(const name& validator, string post_hash, const name& validatee) 
{
   //require_auth( validator );

   day_accounts_table accounts( get_self(), get_self().value );
   auto iterator = accounts.find( validatee.value );
   check( iterator != accounts.end(), "Account could not be found." );

   bool account_is_validated = iterator->is_synced;
   bool hashes_are_equal = post_hash == iterator->voice_post_hash;
   if (!account_is_validated && hashes_are_equal) {
      accounts.modify(iterator, get_self(), [&]( auto& account ) {
         account.is_synced = true;
      });
   }
}

ACTION daycointoken::makeclaim( const name& account_name ) 
{
   //require_auth( account_name );

   daycointoken::makeclaim_validate_account(account_name); // validate that the account_name exists
   daycointoken::makeclaim_record_claimant(account_name); // record claimant if the claim doesn't already exist for the day
   daycointoken::makeclaim_process_claims(account_name); // process claims if it's the next day
}

ACTION daycointoken::clrclaimants()
{
   require_auth( get_self() );

   claimants_table claimants( get_self(), get_self().value );
   auto iterator = claimants.begin( );
   while (iterator != claimants.end()) {
      iterator = claimants.erase(iterator);
   }
}

ACTION daycointoken::clearglobals()
{
   //bool globals_exists = global_properties.exists();
   //print("globals_exists ", globals_exists);
   global_properties.remove();
   bool globals_exists = global_properties.exists();
   print("globals_exists ", globals_exists);
}

ACTION daycointoken::debitdep(const name& account_name, uint64_t deposit_amount) 
{

}

ACTION daycointoken::debitwthdrw(const name& account_name, uint64_t withdrawal_amount) 
{

}

//ACTION daycointoken::stake(const name& account_name, uint64_t stake_amount, timespan_days timespan) {}

ACTION daycointoken::unstake(uint64_t stake_id, const name& account_name, uint64_t withdrawal_amount) 
{

}

ACTION daycointoken::proposalmake(const name& account_name, uint64_t amount, uint64_t number_of_months, const string& ipfs_address, uint64_t ipfs_hash) 
{

}

ACTION daycointoken::proposalvote(const name& account_name, uint64_t proposal_id, uint64_t number_of_votes, bool yes) 
{

}

ACTION daycointoken::create( const name& issuer, const asset& maximum_supply )
{
    require_auth( get_self() );

    auto sym = maximum_supply.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    check( maximum_supply.is_valid(), "invalid supply");
    check( maximum_supply.amount > 0, "max-supply must be positive");

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing == statstable.end(), "token with symbol already exists" );

    statstable.emplace( get_self(), [&]( auto& s ) {
       s.supply.symbol = maximum_supply.symbol;
       s.max_supply    = maximum_supply;
       s.issuer        = issuer;
    });
}

ACTION daycointoken::issue( const name& to, const asset& quantity, const string& memo )
{
   auto sym = quantity.symbol;
   stats statstable( get_self(), sym.code().raw() );
   auto existing = statstable.find( sym.code().raw() );
   check( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
   // I think this is a pointer and I don't get why it's doing it this way?
   const auto& st = *existing;
   require_auth( st.issuer );
   daycointoken::issue_day(to, quantity, memo);
}

void daycointoken::issue_day(const name& to, const asset& quantity, const string& memo )
{
   auto sym = quantity.symbol;
   check( sym.is_valid(), "invalid symbol name" );
   check( memo.size() <= 256, "memo has more than 256 bytes" );

   stats statstable( get_self(), sym.code().raw() );
   auto existing = statstable.find( sym.code().raw() );
   check( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
   const auto& st = *existing;
   check( to == st.issuer, "tokens can only be issued to issuer account" );

   check( quantity.is_valid(), "invalid quantity" );
   check( quantity.amount > 0, "must issue positive quantity" );

   check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
   check( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

   statstable.modify( st, same_payer, [&]( auto& s ) {
      s.supply += quantity;
   });

   add_balance( st.issuer, quantity, st.issuer );
}

ACTION daycointoken::retire( const asset& quantity, const string& memo )
{
    auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "token with symbol does not exist" );
    const auto& st = *existing;

    require_auth( st.issuer );
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must retire positive quantity" );

    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );

    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.supply -= quantity;
    });

    sub_balance( st.issuer, quantity );
}

ACTION daycointoken::transfer( const name& from, const name& to, const asset& quantity, const string& memo )
{
   require_auth(from);
   daycointoken::transfer_day(from, to, quantity, memo);
}

void daycointoken::transfer_day(const name& from, const name& to, const asset& quantity, const string& memo)
{
   check( from != to, "cannot transfer to self" );
   //require_auth( from );
   string to_account_message = "to account (";
   to_account_message += to.to_string();
   to_account_message += ") does not exist.";
   check( is_account( to ), to_account_message);
   auto sym = quantity.symbol.code();
   stats statstable( get_self(), sym.raw() );
   const auto& st = statstable.get( sym.raw() );

   require_recipient( from );
   require_recipient( to );

   check( quantity.is_valid(), "invalid quantity" );
   check( quantity.amount > 0, "must transfer positive quantity" );
   check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
   check( memo.size() <= 256, "memo has more than 256 bytes" );

   auto payer = has_auth( to ) ? to : from;

   sub_balance( from, quantity );
   add_balance( to, quantity, payer );
}

ACTION daycointoken::open( const name& owner, const symbol& symbol, const name& ram_payer )
{
   require_auth( ram_payer );

   check( is_account( owner ), "owner account does not exist" );

   auto sym_code_raw = symbol.code().raw();
   stats statstable( get_self(), sym_code_raw );
   const auto& st = statstable.get( sym_code_raw, "symbol does not exist" );
   check( st.supply.symbol == symbol, "symbol precision mismatch" );

   accounts acnts( get_self(), owner.value );
   auto it = acnts.find( sym_code_raw );
   if( it == acnts.end() ) {
      acnts.emplace( ram_payer, [&]( auto& a ){
        a.balance = asset{0, symbol};
      });
   }
}

ACTION daycointoken::close( const name& owner, const symbol& symbol )
{
   require_auth( owner );
   accounts acnts( get_self(), owner.value );
   auto it = acnts.find( symbol.code().raw() );
   check( it != acnts.end(), "Balance row already deleted or never existed. Action won't have any effect." );
   check( it->balance.amount == 0, "Cannot close because the balance is not zero." );
   acnts.erase( it );
}

void daycointoken::sub_balance( const name& owner, const asset& value ) {
   accounts from_acnts( get_self(), owner.value );

   const auto& from = from_acnts.get( value.symbol.code().raw(), "no balance object found" );
   string overdrawn_message = "overdrawn balance - ";
   overdrawn_message += "from.balance.amount: ";
   overdrawn_message += to_string(from.balance.amount);
   overdrawn_message += "; value.amount: ";
   overdrawn_message += to_string(value.amount);
   check( from.balance.amount >= value.amount, overdrawn_message );

   from_acnts.modify( from, owner, [&]( auto& a ) {
         a.balance -= value;
      });
}

void daycointoken::add_balance( const name& owner, const asset& value, const name& ram_payer )
{
   accounts to_acnts( get_self(), owner.value );
   auto to = to_acnts.find( value.symbol.code().raw() );
   if( to == to_acnts.end() ) {
      to_acnts.emplace( ram_payer, [&]( auto& a ){
        a.balance = value;
      });
   } else {
      to_acnts.modify( to, same_payer, [&]( auto& a ) {
        a.balance += value;
      });
   }
}

void daycointoken::makeclaim_validate_account( const name& account_name )
{
   day_accounts_table dayaccounts( get_self(), get_self().value );
   auto dayaccounts_iterator = dayaccounts.find( account_name.value );
   check( dayaccounts_iterator != dayaccounts.end(), "Account does not exist." );
   check( dayaccounts_iterator->is_synced, "Account must by synced with Voice to make claim." );
}

void daycointoken::makeclaim_record_claimant( const name& account_name )
{
   auto globals = global_properties.get_or_create(get_self(), globalsrow);
   
   //globals.current_days_claimants_count = globals.current_days_claimants_count + 1;
   global_properties.set(globals, get_self());

   claimants_table claimants( get_self(), get_self().value );
   auto claimants_iterator = claimants.find( account_name.value );
   if (claimants_iterator == claimants.end() ) {
      // this account has not claimed today
      claimants.emplace( get_self(), [&]( auto& claimant ) {
         claimant.claim_day = globals.current_day;
         claimant.claimant = account_name;
      });
   }
}

void daycointoken::makeclaim_process_claims( const name& account_name )
{
   require_auth( account_name );

   auto globals = global_properties.get();

   uint64_t seconds_since_epoch = current_time_point().sec_since_epoch();

   bool is_past_next_processed_time = 
      globals.last_time_processed == 0 || 
      seconds_since_epoch > (globals.last_time_processed + globals.claim_span_seconds);

   uint64_t current_day = globals.current_day;

   if (is_past_next_processed_time) {

      uint64_t claimants_count = 0;

      claimants_table claimants( get_self(), get_self().value );
      auto claimants_by_day = claimants.get_index< "byday"_n >();

      auto claimants_lower = claimants_by_day.lower_bound(current_day);
      auto claimants_upper = claimants_by_day.upper_bound(current_day);
      for ( auto i = claimants_lower; i != claimants_upper; i++ ) {
         claimants_count++;
      }

      uint64_t wps_amount = 0;
      uint64_t erps_amount = 0;
      uint64_t winner_amount = 0;
      uint64_t slice_size = 0;

      if (claimants_count < 10) {
         slice_size = globals.issue_amount / (claimants_count + 3); // 3 is for the wps, erps, and winner
         wps_amount = slice_size;
         erps_amount = slice_size;
         winner_amount = slice_size;
      } else {
         wps_amount = globals.issue_amount * globals.wps_percent;
         erps_amount = globals.issue_amount * globals.erps_percent;
         slice_size = (globals.issue_amount - wps_amount - erps_amount) / (claimants_count + 1); // 1 is for winner
         winner_amount = slice_size;
      }

      print(" slice size: ", slice_size);

      uint64_t slice_total = (slice_size * claimants_count) + wps_amount + erps_amount + winner_amount;

      claimants_lower = claimants_by_day.lower_bound(current_day);
      claimants_upper = claimants_by_day.upper_bound(current_day);
      uint64_t processed_claim_day = current_day;
      current_day++;
      globals.current_day = current_day;
      globals.last_time_processed = seconds_since_epoch;
      global_properties.set(globals, get_self());
      
      /* 
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      ////// for whatever reason I cannot call the inline issue action so I've written my own helper function that issues DAY /////////
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      action(
         permission_level { "daycointoken"_n, "active"_n },
         "daycointoken"_n,
         "issue"_n,
         make_tuple( "daycointoken"_n, asset(slice_total, symbol(symbol_code("DAY"), 13)), "")
      ).send();*/
      daycointoken::issue_day( "daycointoken"_n, asset(slice_total, symbol(symbol_code("DAY"), 13)), "" );
      
      for ( auto i = claimants_lower; i != claimants_upper; i++ ) {
         print(i->claimant, ", ");
         /*action(
            permission_level { get_self(), "active"_n },
            get_self(),
            "transfer"_n,
            make_tuple( get_self(), i->claimant, asset(slice_size, symbol(symbol_code("DAY"), 13)), "CLAIMANT PAYMENT" )
         ).send();*/
         uint64_t slice_amount = slice_size;
         print("; slice amount: ", slice_amount);
         if (i->claimant == account_name) {
            slice_amount = slice_amount + winner_amount;
            print("; winner amount: ", slice_amount);
         }
         daycointoken::transfer_day( "daycointoken"_n, i->claimant, asset(slice_amount, symbol(symbol_code("DAY"), 13)), "CLAIMANT PAYMENT" );
      }
      /*action(
         permission_level { get_self(), "active"_n },
         get_self(),
         "transfer"_n,
         make_tuple( get_self(), "daycoinerps1"_n, asset(erps_amount, symbol(symbol_code("DAY"), 13)), "CLAIMANT PAYMENT" )
      ).send();*/
      daycointoken::transfer_day( "daycointoken"_n, "daycoinerps1"_n, asset(erps_amount, symbol(symbol_code("DAY"), 13)), "CLAIMANT PAYMENT" );
      /*action(
         permission_level { get_self(), "active"_n },
         get_self(),
         "transfer"_n,
         make_tuple( get_self(), "daycoinwpsio"_n, asset(wps_amount, symbol(symbol_code("DAY"), 13)), "CLAIMANT PAYMENT" )
      ).send();*/
      daycointoken::transfer_day( "daycointoken"_n, "daycoinwpsio"_n, asset(wps_amount, symbol(symbol_code("DAY"), 13)), "CLAIMANT PAYMENT" );

      daycointoken::record_winning_claim( 
         processed_claim_day,
         account_name,
         slice_size,
         claimants_count,
         winner_amount,
         wps_amount, 
         erps_amount, 
         slice_total );
         //uint64_t issuer_balance );

      auto iterator = claimants.begin( );
      while (iterator != claimants.end()) {
         iterator = claimants.erase(iterator);
      }

      daycointoken::makeclaim_validate_account(account_name); // validate that the account_name exists
      daycointoken::makeclaim_record_claimant(account_name); // record claimant if the claim doesn't already exist for the day
   }
}

void daycointoken::record_winning_claim( 
   uint64_t claim_day,
   name winning_claim,
   uint64_t slice_size,
   uint64_t claimant_count,
   uint64_t winner_bonus_amount,
   uint64_t wps_amount, 
   uint64_t erps_amount, 
   uint64_t slice_total )
{
   claims_table claims( get_self(), get_self().value );
   auto iterator = claims.find( claim_day );
   check( iterator == claims.end(), "Claim already recorded for this day." );

   claims.emplace( get_self(), [&]( auto& claim ) {
      claim.claim_day = claim_day;
      claim.winning_claim = winning_claim;
      claim.slice_size = slice_size;
      claim.claimant_count = claimant_count;
      claim.winner_bonus_amount = winner_bonus_amount;
      claim.wps_amount = wps_amount;
      claim.erps_amount = erps_amount;
      claim.slice_total = slice_total;
   });
}


EOSIO_DISPATCH(daycointoken, 
    (registeracct)(makeclaim)(debitdep)(debitwthdrw)(unstake)(proposalmake)(proposalvote)(clearallacct)(valvoiceacct)(clrclaimants)(clearglobals)//(stake)
    (create)(issue)(retire)(transfer)(open)(close))
