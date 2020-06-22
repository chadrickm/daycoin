#ifndef CLAIMANT
#define CLAIMANT

class [[eosio::table, eosio::contract("daycoinclaim")]] claimant_t {

    private:
        
        uint64_t claim_day;
        eosio::name claimant;

    public:
        
        // ctor that does nothing
        claimant_t() {}

        // ctor for setting each prop
        claimant_t(
            uint64_t const _claim_day,
            eosio::name const & _claimant) :
            claim_day(_claim_day),
            claimant(_claimant) {}
        
        uint64_t const get_claim_day() const { return claim_day; }
        eosio::name const get_claimant() const { return claimant; }

        uint64_t primary_key() const { return get_claimant().value; }

        EOSLIB_SERIALIZE( claimant_t, (claim_day)(claimant) )
};

typedef eosio::multi_index< "claimants"_n, claimant_t > claimants_table;

#endif