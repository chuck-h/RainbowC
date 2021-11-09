#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/system.hpp>

#include <string>

namespace eosio {

   using std::string;

   /**
    * The `rainbow` experimental contract implements the functionality described in the design document
    * https://rieki-cordon.medium.com/1fb713efd9b1 .
    * In the development process we are building on the eosio.token code. 
    *
    * The token contract defines the structures and actions that allow users to create, issue, and manage tokens for EOSIO based blockchains. It exemplifies one way to implement a smart contract which allows for creation and management of tokens.
    * 
    * The `rainbow` contract class also implements two useful public static methods: `get_supply` and `get_balance`. The first allows one to check the total supply of a specified token, created by an account and the second allows one to check the balance of a token for a specified account (the token creator account has to be specified as well).
    * 
    * The `rainbow` contract manages the set of tokens, stakes, accounts and their corresponding balances, by using four internal multi-index structures: the `accounts`, `stats`, `configs`, and `stakes`. The `accounts` multi-index table holds, for each row, instances of `account` object and the `account` object holds information about the balance of one token. The `accounts` table is scoped to an eosio account, and it keeps the rows indexed based on the token's symbol.  This means that when one queries the `accounts` multi-index table for an account name the result is all the tokens that account holds at the moment.
    * 
    * Similarly, the `stats` multi-index table, holds instances of `currency_stats` objects for each row, which contains information about current supply, maximum supply, and the creator account. The `stats` table is scoped to the token symbol_code. Therefore, when one queries the `stats` table for a token symbol the result is one single entry/row corresponding to the queried symbol token if it was previously created, or nothing, otherwise.
    *
    * The first two tables (`accounts` and `stats`) are structured identically to the `eosio.token` tables, making "rainbow tokens" compatible with most EOSIO wallet and block explorer applications. The two remaining tables (`configs` and `stakes`) provide additional data specific to the rainbow token.
    *
    * The `configs` table contains names of administration accounts (e.g. membership_mgr, freeze_mgr) and some configuration flags. The `configs` table is scoped to the token symbol_code and has a single row per scope.
    *
    * The `stakes` table contains staking relationships (staked currency, staking ratio, escrow account). It is scoped by the token symbol_code and may contain 1 or more rows. It has a secondary index based on the staked currency type.
    */

   class [[eosio::contract("rainbowtoken")]] token : public contract {
      public:
         using contract::contract;

         /**
          * The ` create` action allows `issuer` account to create or reconfigure a token with the
          * specified characteristics. 
          * If the token does not exist, a new row in the stats table for token symbol scope is created
          * with the specified characteristics. At creation, its' approval flag is false, preventing
          * tokens from being issued. If a token of this symbol does exist and update
          * is permitted, the characteristics are updated.
          *
          * @param issuer - the account that creates the token,
          * @param maximum_supply - the maximum supply set for the token,
          * @param membership_mgr - the account with authority to whitelist accounts to send tokens,
          * @param withdrawal_mgr - the account with authority to withdraw tokens from any account,
          * @param withdraw_to - the account to which withdrawn tokens are deposited,
          * @param freeze_mgr - the account with authority to freeze transfer actions,
          * @param bearer_redeem - a boolean allowing token holders to redeem the staked dSeeds,
          * @param config_locked_until - an ISO8601 date string; changes to token characteristics
          *   are disallowed until this time; blank string is equivalent to "now" (i.e. unlocked).
          *
          * @pre Token symbol has to be valid,
          * @pre Token symbol must not be already created, OR if it has been created,
          *   the config_locked field in the configtable row must be in the past,
          * @pre maximum_supply has to be smaller than the maximum supply allowed by the system: 2^62 - 1.
          * @pre Maximum supply must be positive,
          * @pre membership manager must be an existing account,
          * @pre withdrawal manager must be an existing account,
          * @pre withdraw_to must be an existing account,
          * @pre freeze manager must be an existing account,
          * @pre membership manager must be an existing account;
          * @pre config_locked_until must specify a time within +100/-10 yrs of now;
          */
         [[eosio::action]]
         void create( const name&   issuer,
                      const asset&  maximum_supply,
                      const name&   membership_mgr,
                      const name&   withdrawal_mgr,
                      const name&   withdraw_to,
                      const name&   freeze_mgr,
                      const bool&   bearer_redeem,
                      const string& config_locked_until);


         /**
          * By this action the contract owner approves the creation of the token. Until
          * this approval, no tokens may be issued.
          *
          * @param symbolcode - the symbol_code of the token to execute the close action for.
          *
          * @pre The symbol must have been created.
          */
         [[eosio::action]]
         void approve( const symbol_code& symbolcode );


         /**
          * Allows `issuer` account to create or reconfigure a staking relationship for a token. If 
          * the relationship does not exist, a new entry in the stakes table for token symbol scope gets created. If there
          * is no row of the stakes table in that scope associated with the issuer, a new row gets created
          * with the specified characteristics. If a token of this symbol and issuer does exist and update
          * is permitted, the characteristics are updated.
          *
          * @param issuer - the account that created the token,
          * @param token_bucket - a reference quantity of the token,
          * @param stake_per_bucket - the number of stake tokens (e.g. Seeds) staked per "bucket" of tokens,
          * @param stake_token_contract - the staked token contract account (e.g. token.seeds),
          * @param stake_to - the escrow account where stake is held, or `deletestake`
          *   to remove a row from the stakes table
          * @param memo - the memo string to accompany the transaction.
          *
          * @pre Token symbol must have already been created by this issuer
          * @pre The config_locked_until field in the configs table must be in the past,
          * @pre issuer must have a (possibly zero) balance of the stake token,
          * @pre stake_per_bucket must be non-negative
          * @pre issuer active permissions must include rainbowcontract@eosio.code
          * @pre stake_to active permissions must include rainbowcontract@eosio.code
          */
         [[eosio::action]]
         void setstake( const name&   issuer,
                        const asset&  token_bucket,
                        const asset&  stake_per_bucket,
                        const name&   stake_token_contract,
                        const name&   stake_to,
                        const string& memo);
         /**
          *  This action issues a `quantity` of tokens to the issuer account.
          *
          * @param to - the account to issue tokens to, it must be the same as the issuer,
          * @param quantity - the amount of tokens to be issued,
          * @memo - the memo string that accompanies the token issue transaction.
          * 
          * @pre The `approve` action must have been executed for this token symbol
          */
         [[eosio::action]]
         void issue( const asset& quantity, const string& memo );

         /**
          * The opposite for issue action, if all validations succeed,
          * it debits the statstable.supply amount. Any staked tokens are released from escrow in
          * proportion to the quantity of tokens retired.
          *
          * @param owner - the account containing tokens to retire,
          * @param quantity - the quantity of tokens to retire,
          * @param memo - the memo string to accompany the transaction.
          */
         [[eosio::action]]
         void retire( const name& owner, const asset& quantity, const string& memo );

         /**
          * Allows `from` account to transfer to `to` account the `quantity` tokens.
          * One account is debited and the other is credited with quantity tokens.
          *
          * @param from - the account to transfer from,
          * @param to - the account to be transferred to,
          * @param quantity - the quantity of tokens to be transferred,
          * @param memo - the memo string to accompany the transaction.
          * 
          * @pre The transfers_frozen flag in the configs table must be false, except for
          *   administrative-account transfers
          */
         [[eosio::action]]
         void transfer( const name&    from,
                        const name&    to,
                        const asset&   quantity,
                        const string&  memo );
         /**
          * Allows `ram_payer` to create an account `owner` with zero balance for
          * token `symbolcode` at the expense of `ram_payer`.
          *
          * @param owner - the account to be created,
          * @param symbolcode - the token symbol,
          * @param ram_payer - the account that supports the cost of this action.
          *
          * More information can be read [here](https://github.com/EOSIO/eosio.contracts/issues/62)
          * and [here](https://github.com/EOSIO/eosio.contracts/issues/61).
          */
         [[eosio::action]]
         void open( const name& owner, const symbol_code& symbolcode, const name& ram_payer );

         /**
          * This action is the opposite for open, it closes the account `owner`
          * for token `symbol`.
          *
          * @param owner - the owner account to execute the close action for,
          * @param symbolcode - the symbol of the token to execute the close action for.
          *
          * @pre The pair of owner plus symbol has to exist otherwise no action is executed,
          * @pre If the pair of owner plus symbol exists, the balance has to be zero.
          */
         [[eosio::action]]
         void close( const name& owner, const symbol_code& symbolcode );

         /**
          * This action freezes or unfreezes transaction processing
          * for token `symbol`.
          *
          * @param symbol - the symbol of the token to execute the freeze action for.
          * @param freeze - boolean, true = freeze, false = enable transfers.
          * @param memo - the memo string to accompany the transaction.
          *
          * @pre The symbol has to exist otherwise no action is executed,
          * @pre Transaction must have the freeze_mgr authority 
          */
         [[eosio::action]]
         void freeze( const symbol_code& symbolcode, const bool& freeze, const string& memo );

         /**
          * This action clears a RAM table (development use only!)
          *
          * @param table - name of table
          * @param scope - string
          * @param limit - max number of erasures (for time control)
          *
          * @pre Transaction must have the contract account authority 
          */
         [[eosio::action]]
         void resetram( const name& table, const string& scope, const uint32_t& limit = 10 );

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

         using create_action = eosio::action_wrapper<"create"_n, &token::create>;
         using approve_action = eosio::action_wrapper<"approve"_n, &token::approve>;
         using setstake_action = eosio::action_wrapper<"setstake"_n, &token::setstake>;
         using issue_action = eosio::action_wrapper<"issue"_n, &token::issue>;
         using retire_action = eosio::action_wrapper<"retire"_n, &token::retire>;
         using transfer_action = eosio::action_wrapper<"transfer"_n, &token::transfer>;
         using open_action = eosio::action_wrapper<"open"_n, &token::open>;
         using close_action = eosio::action_wrapper<"close"_n, &token::close>;
         using freeze_action = eosio::action_wrapper<"freeze"_n, &token::freeze>;
         using resetram_action = eosio::action_wrapper<"resetram"_n, &token::resetram>;
      private:
         const name allowallacct = "allowallacct"_n;
         const name deletestakeacct = "deletestake"_n;
         const int max_stake_count = 5;
         struct [[eosio::table]] account {
            asset    balance;

            uint64_t primary_key()const { return balance.symbol.code().raw(); }
         };

         struct [[eosio::table]] currency_stats {
            asset    supply;
            asset    max_supply;
            name     issuer;

            uint64_t primary_key()const { return supply.symbol.code().raw(); }
         };

         struct [[eosio::table]] currency_config {
            name       membership_mgr;
            name       withdrawal_mgr;
            name       withdraw_to;
            name       freeze_mgr;
            bool       bearer_redeem;
            time_point config_locked_until;
            bool       transfers_frozen;
            bool       approved;

            uint64_t primary_key()const { return 0; } // single row per scope
         };

         struct [[eosio::table]] stake_stats {
            uint64_t index;
            asset    token_bucket;
            asset    stake_per_bucket;
            name     stake_token_contract;
            name     stake_to;

            uint64_t primary_key()const { return index; };
            uint128_t by_secondary() const {
               return (uint128_t)stake_per_bucket.symbol.raw()<<64 | stake_token_contract.value;
            }
         };

         typedef eosio::multi_index< "accounts"_n, account > accounts;
         typedef eosio::multi_index< "stat"_n, currency_stats > stats;
         typedef eosio::multi_index< "configs"_n, currency_config > configs;
         typedef eosio::multi_index
            < "stakes"_n, stake_stats, indexed_by
               < "staketoken"_n,
                 const_mem_fun<stake_stats, uint128_t, &stake_stats::by_secondary >
               >
            > stakes;

         void sub_balance( const name& owner, const asset& value );
         void add_balance( const name& owner, const asset& value, const name& ram_payer );
         void stake_all( const currency_stats st, const uint64_t amount );
         void unstake_all( const currency_stats st, const name& owner, const uint64_t amount );
         void stake_one( const currency_stats st, const stake_stats sk, const uint64_t amount );
         void unstake_one( const currency_stats st, const stake_stats sk, const name& owner, const uint64_t amount );
 
   };

}
