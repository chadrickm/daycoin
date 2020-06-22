#ifndef ACCOUNT
#define ACCOUNT

class [[eosio::table, eosio::contract("daycoinissue")]] account_t {


    private:
        
        eosio::name         user;
        eosio::time_point   registered_on_timestamp;
        bool                is_unique_account_linked;
        std::string         unique_account_id;


    public:
        
        // ctor that does nothing
        account_t() {}

        // ctor for setting each prop
        account_t(
            eosio::name         _user,
            eosio::time_point   _registered_on_timestamp,
            bool                _is_unique_account_linked,
            std::string         _unique_account_id) 
            :
            user(_user),
            registered_on_timestamp(_registered_on_timestamp),
            is_unique_account_linked(_is_unique_account_linked),
            unique_account_id(_unique_account_id)
        {}
        
        eosio::name const get_user() const { return user; }
        eosio::time_point const get_registered_on_timestamp() const { return registered_on_timestamp; }
        bool const get_is_unique_account_linked() const { return is_unique_account_linked; }
        std::string const get_unique_account_id() const { return unique_account_id; }

        uint64_t primary_key() const { return get_user().value; }

        EOSLIB_SERIALIZE( account_t, 
            (user)(registered_on_timestamp)(is_unique_account_linked)(unique_account_id) )
};

typedef eosio::multi_index< "accounts"_n, account_t > accounts_table;

#endif