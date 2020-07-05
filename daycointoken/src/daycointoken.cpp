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
   require_auth( validator );

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
   require_auth( account_name );

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
    require_auth( name("daycoinadmin") );

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
    check( sym.is_valid(), "invalid symbol name" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
    const auto& st = *existing;
    check( to == st.issuer, "tokens can only be issued to issuer account" );

    require_auth( st.issuer );
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
    check( from != to, "cannot transfer to self" );
    require_auth( from );
    check( is_account( to ), "to account does not exist");
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
   check( from.balance.amount >= value.amount, "overdrawn balance" );

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
   
   // if this is the first time we are running the app
   if (globals.current_day == 0) {
      globals.current_day = 1;
   }
   
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

   bool is_tomorrow = true;
   if (is_tomorrow) {
      uint64_t claimants_count = 0;
      claimants_table claimants( get_self(), get_self().value );
      auto claimants_by_day = claimants.get_index< "byday"_n >();
      auto claimants_lower = claimants_by_day.lower_bound(globals.current_day);
      auto claimants_upper = claimants_by_day.upper_bound(globals.current_day);
      for ( auto i = claimants_lower; i != claimants_upper; i++ ) {
         claimants_count++;
      }

      float wps = .1;
      float erps = .1;
      float slice_size = 0.0;
      if (claimants_count < 10) {
         slice_size = floorf( ( 1.0 / (claimants_count + 2) ) * 1000000000000000000 ) / 1000000000000000000;
         wps = slice_size;
         erps = slice_size;
      } else {
         slice_size = floorf( ( .8 / claimants_count ) * 1000000000000000000 ) / 1000000000000000000;
      }

      float slice_total = (slice_size * claimants_count) + wps + erps;

      //print("slice_total ", slice_total);

      claimants_lower = claimants_by_day.lower_bound(globals.current_day);
      claimants_upper = claimants_by_day.upper_bound(globals.current_day);
      globals.current_day++;
      global_properties.set(globals, get_self());
      for ( auto i = claimants_lower; i != claimants_upper; i++ ) {
         print(i->claimant, ", ");
         // TODO: issue slice_size DAY to each claimant account
      }

      auto iterator = claimants.begin( );
      while (iterator != claimants.end()) {
         iterator = claimants.erase(iterator);
      }

      daycointoken::makeclaim_validate_account(account_name); // validate that the account_name exists
      daycointoken::makeclaim_record_claimant(account_name); // record claimant if the claim doesn't already exist for the day
   }
}


EOSIO_DISPATCH(daycointoken, 
    (registeracct)(makeclaim)(debitdep)(debitwthdrw)(unstake)(proposalmake)(proposalvote)(clearallacct)(valvoiceacct)(clrclaimants)//(stake)
    (create)(issue)(retire)(transfer)(open)(close))
