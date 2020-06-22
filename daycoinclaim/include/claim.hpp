#ifndef CLAIM
#define CLAIM

class [[eosio::table, eosio::contract("daycoinclaim")]] claim_t {

    private:
        
        uint64_t            claim_day;
        eosio::name         winning_claimant;
        eosio::time_point   claim_timestamp;
        uint64_t            seconds_since_epoch;
        uint64_t            seconds_since_last_claim;
        uint64_t            seconds_until_next_claim;

    public:
        
        // ctor that does nothing
        claim_t() {}

        // ctor for setting each prop
        claim_t(
            uint64_t const _claim_day,
            eosio::name const & _winning_claimant,
            eosio::time_point const _claim_timestamp,
            uint64_t const _seconds_since_epoch,
            uint64_t const _seconds_since_last_claim,
            uint64_t const _seconds_until_next_claim) 
            :
            claim_day(_claim_day),
            winning_claimant(_winning_claimant),
            claim_timestamp(_claim_timestamp),
            seconds_since_epoch(_seconds_since_epoch),
            seconds_since_last_claim(_seconds_since_last_claim),
            seconds_until_next_claim(_seconds_until_next_claim) {}
        
        uint64_t const get_claim_day() const { return claim_day; }
        eosio::name const get_winning_claimant() const { return winning_claimant; }
        eosio::time_point const get_claim_timestamp() const { return claim_timestamp; }
        uint64_t const get_seconds_since_epoch() const { return seconds_since_epoch; }
        uint64_t const get_seconds_since_last_claim() const { return seconds_since_last_claim; }
        uint64_t const get_seconds_until_next_claim() const { return seconds_until_next_claim; }

        uint64_t primary_key() const { return get_claim_day(); }

        EOSLIB_SERIALIZE( claim_t, 
            (claim_day)(winning_claimant)(claim_timestamp)(seconds_since_epoch)(seconds_until_next_claim) )
};

typedef eosio::multi_index< "claims"_n, claim_t > claims_table;

#endif