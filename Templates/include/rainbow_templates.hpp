#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <eosio/system.hpp>

#include <string>

using namespace eosio;

   using std::string;


   CONTRACT rbtemplates : public contract {
      public:
         using contract::contract;
         rbtemplates( name receiver, name code, datastream<const char*> ds )
         : contract( receiver, code, ds )
         {}

         ACTION importer(const name& caller, const name& contract, const symbol_code& symbolcode, const string& table );

         ACTION erase(const symbol_code& symbolcode, const string& table );


      private:

         TABLE currency_stats {  // scoped on token symbol code
            asset    supply;
            asset    max_supply;
            name     issuer;

            uint64_t primary_key()const { return supply.symbol.code().raw(); }
         };

         TABLE currency_config {  // singleton, scoped on token symbol code
            name        membership_mgr;
            name        withdrawal_mgr;
            name        withdraw_to;
            name        freeze_mgr;
            time_point  redeem_locked_until;
            time_point  config_locked_until;
            bool        transfers_frozen;
            bool        approved;
            symbol_code cred_limit;
            symbol_code positive_limit;
         };

         TABLE currency_display {  // singleton, scoped on token symbol code
            string     json_meta;
         };


         TABLE stake_stats {  // scoped on token symbol code
            uint64_t index;
            asset    token_bucket;
            asset    stake_per_bucket;
            name     stake_token_contract;
            name     stake_to;
            bool     proportional;

            uint64_t primary_key()const { return index; };
            uint128_t by_secondary() const {
               return (uint128_t)stake_per_bucket.symbol.raw()<<64 | stake_token_contract.value;
            }
         };

         TABLE symbolt { // scoped on get_self()
            symbol_code  symbolcode;

            uint64_t primary_key()const { return symbolcode.raw(); };
         };

         typedef eosio::multi_index< "stat"_n, currency_stats > stats;
         typedef eosio::singleton< "configs"_n, currency_config > configs;
         typedef eosio::multi_index< "configs"_n, currency_config >  dump_for_config;
         typedef eosio::singleton< "displays"_n, currency_display > displays;
         typedef eosio::multi_index< "displays"_n, currency_display >  dump_for_display;
         typedef eosio::multi_index< "stakes"_n, stake_stats, indexed_by
               < "staketoken"_n,
                 const_mem_fun<stake_stats, uint128_t, &stake_stats::by_secondary >
               >
            > stakes;

         static const std::vector<string> tablenames;

   };

EOSIO_DISPATCH( rbtemplates, (importer)(erase) );

