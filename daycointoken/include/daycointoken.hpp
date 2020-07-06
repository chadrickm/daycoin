#include <eosio/eosio.hpp>
//#include <timespan.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>

#include <string>
#include <math.h>

using namespace std;
using namespace eosio;

CONTRACT daycointoken : public contract {
  public:
    using contract::contract;

    daycointoken( name receiver, name code, datastream<const char*> ds ) :
         contract(receiver, code, ds), global_properties(receiver, receiver.value) {}

    // DayCoin Ecosystem Actions
    ACTION registeracct(const name& account_name, const string& voice_account_name);
    ACTION clearallacct();
    ACTION valvoiceacct(const name& validator, string post_hash, const name& validatee);
    ACTION makeclaim(const name& account_name);
    ACTION clrclaimants();
    ACTION clearglobals();
    ACTION debitdep(const name& account_name, uint64_t deposit_amount);
    ACTION debitwthdrw(const name& account_name, uint64_t withdrawal_amount);
    //ACTION stake(const name& account_name, uint64_t stake_amount, timespan_days timespan);
    ACTION unstake(uint64_t stake_id, const name& account_name, uint64_t withdrawal_amount);
    ACTION proposalmake(const name& account_name, uint64_t amount, uint64_t number_of_months, const string& ipfs_address, uint64_t ipfs_hash);
    ACTION proposalvote(const name& account_name, uint64_t proposal_id, uint64_t number_of_votes, bool yes);

    // EOSIO.Token Actions
    ACTION create( const name& issuer, const asset& maximum_supply);
    ACTION issue( const name& to, const asset& quantity, const string& memo );
    ACTION retire( const asset& quantity, const string& memo );
    ACTION transfer( const name& from, const name& to, const asset& quantity, const string&  memo );
    ACTION open( const name& owner, const symbol& symbol, const name& ram_payer );
    ACTION close( const name& owner, const symbol& symbol );
    static asset get_supply( const name& token_contract_account, const symbol_code& sym_code )
    {
      stats statstable( token_contract_account, sym_code.raw() );
      const auto& st = statstable.get( sym_code.raw() );
      return st.supply;
    }
    static asset get_balance( const name& token_contract_account, const name& owner, const symbol_code& sym_code )
    {
      accounts accountstable( token_contract_account, owner.value );
      const auto& ac = accountstable.get( sym_code.raw() );
      return ac.balance;
    }
    using create_action = eosio::action_wrapper<"create"_n, &daycointoken::create>;
    using issue_action = eosio::action_wrapper<"issue"_n, &daycointoken::issue>;
    using retire_action = eosio::action_wrapper<"retire"_n, &daycointoken::retire>;
    using transfer_action = eosio::action_wrapper<"transfer"_n, &daycointoken::transfer>;
    using open_action = eosio::action_wrapper<"open"_n, &daycointoken::open>;
    using close_action = eosio::action_wrapper<"close"_n, &daycointoken::close>;

  private:

    TABLE globals {
      uint64_t current_day;
      uint64_t last_time_processed;
    } globalsrow;
    using singleton_type = eosio::singleton<"globals"_n, globals>;
    singleton_type global_properties;

    TABLE claims {
      uint64_t claim_day;
      name winning_claim;
      uint64_t slice_size;
      uint64_t claimant_count;
      uint64_t winner_bonus_amount;
      uint64_t wps_amount;
      uint64_t erps_amount;
      uint64_t slice_total;
      uint64_t issuer_balance;

      uint64_t primary_key() const { return claim_day; }
    };
    typedef eosio::multi_index< "claims"_n, claims > claims_table;

    TABLE claimants {
      uint64_t claim_day;
      eosio::name claimant;

      uint64_t primary_key() const { return claimant.value; }

      uint64_t get_claim_day() const { return claim_day; }
    };
    typedef eosio::multi_index< "claimants"_n, claimants, 
      indexed_by< "byday"_n, const_mem_fun< claimants, uint64_t, &claimants::get_claim_day > > 
      > claimants_table;


    TABLE day_accounts {
      name account_name;
      string voice_account_name;
      string voice_post_hash;
      bool is_synced;

      uint64_t primary_key()const { return account_name.value; }
    };
    typedef eosio::multi_index< "dayaccounts"_n, day_accounts > day_accounts_table;


    TABLE account {
      asset    balance;

      uint64_t primary_key()const { return balance.symbol.code().raw(); }
    };
    typedef eosio::multi_index< "accounts"_n, account > accounts;


    TABLE currency_stats {
      asset    supply;
      asset    max_supply;
      name     issuer;

      uint64_t primary_key()const { return supply.symbol.code().raw(); }
    };
    typedef eosio::multi_index< "stat"_n, currency_stats > stats;

    void issue_day( const name& to, const asset& quantity, const string& memo );
    void transfer_day(const name& from, const name& to, const asset& quantity, const string& memo);
    void sub_balance( const name& owner, const asset& value );
    void add_balance( const name& owner, const asset& value, const name& ram_payer );
    void record_winning_claim( 
      uint64_t claim_day,
      name winning_claim,
      uint64_t slice_size,
      uint64_t claimant_count,
      uint64_t winner_bonus_amount,
      uint64_t wps_amount, 
      uint64_t erps_amount, 
      uint64_t slice_total ); 
      //uint64_t issuer_balance );
    
    void makeclaim_validate_account( const name& account_name );
    void makeclaim_record_claimant( const name& account_name );
    void makeclaim_process_claims( const name& account_name );

};
