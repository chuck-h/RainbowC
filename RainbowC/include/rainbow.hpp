#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>

#include <string>

namespace eosio {

   using std::string;

   /**
    * The `rainbowtoken` experimental contract implements the functionality described in the design document
    * https://rieki-cordon.medium.com/1fb713efd9b1 .
    * In the development process we are building on the eosio.token code. During development many original inline comments
    * will continue to refer to `eosio.token`.
    *
    * The `eosio.token` sample system contract defines the structures and actions that allow users to create, issue, and manage tokens for EOSIO based blockchains. It demonstrates one way to implement a smart contract which allows for creation and management of tokens. It is possible for one to create a similar contract which suits different needs. 
    * 
    * The `rainbowtoken` contract class also implements two useful public static methods: `get_supply` and `get_balance`. The first allows one to check the total supply of a specified token, created by an account and the second allows one to check the balance of a token for a specified account (the token creator account has to be specified as well).
    * 
    * The `rainbowtoken` contract manages the set of tokens, accounts and their corresponding balances, by using two internal multi-index structures: the `accounts` and `stats`. The `accounts` multi-index table holds, for each row, instances of `account` object and the `account` object holds information about the balance of one token. The `accounts` table is scoped to an eosio account, and it keeps the rows indexed based on the token's symbol.  This means that when one queries the `accounts` multi-index table for an account name the result is all the tokens that account holds at the moment.
    * 
    * Similarly, the `stats` multi-index table, holds instances of `currency_stats` objects for each row, which contains information about current supply, maximum supply, the creator account, the freeze status, and a variety of configured parameters for a symbol token. The `stats` table is scoped to the token symbol. Therefore, when one queries the `stats` table for a token symbol the result is one single entry/row corresponding to the queried symbol token if it was previously created, or nothing, otherwise.
    */

   class [[eosio::contract("rainbowtoken")]] token : public contract {
      public:
         using contract::contract;

         /**
          * Allows `issuer` account to create or reconfigure a token with the specified characteristics. If 
          * the token does not exist, a new entry in statstable for token symbol scope gets created. If there
          * is no row of the statstabe in that scope associated with the issuer, a new row gets created
          * with the specified characteristics. If a token of this symbol and issuer does exist and update
          * is permitted, the characteristics are updated.
          *
          * @param issuer - the account that creates the token,
          * @param maximum_supply - the maximum supply set for the token,
          * @param staking_ratio - the number of dSeeds staked per token,
          * @param membership_mgr - the account with authority to whitelist accounts to send tokens,
          * @param withdrawal_mgr - the account with authority to withdraw tokens from any account,
          * @param withdraw_to - the account to which withdrawn tokens are deposited,
          * @param freeze_mgr - the account with authority to freeze transfer actions,
          * @param bearer_redeem - a boolean allowing token holders to redeem the staked dSeeds,
          * @param config_locked - a boolean prohibiting changes to token characteristics.
          *
          * @pre Token symbol has to be valid,
          * @pre Token symbol must not be already created by this issuer, OR if it has been created,
          *   the config_locked field in the statstable row must be false,
          * @pre maximum_supply has to be smaller than the maximum supply allowed by the system: 1^62 - 1.
          * @pre Maximum supply must be positive,
          * @pre staking ratio must be in a valid range
          * @pre membership manager must be an existing account,
          * @pre withdrawal manager must be an existing account,
          * @pre withdraw_to must be an existing account,
          * @pre freeze manager must be an existing account,
          * @pre membership manager must be an existing account;
          */
         [[eosio::action]]
         void create( const name&   issuer,
                      const asset&  maximum_supply,
                      const float&  staking_ratio,
                      const name&   membership_mgr,
                      const name&   withdrawal_mgr,
                      const name&   withdraw_to,
                      const name&   freeze_mgr,
                      const bool&   bearer_redeem,
                      const bool&   config_locked);
         /**
          *  This action issues to `to` account a `quantity` of tokens.
          *
          * @param to - the account to issue tokens to, it must be the same as the issuer,
          * @param quantity - the amount of tokens to be issued,
          * @memo - the memo string that accompanies the token issue transaction.
          */
         [[eosio::action]]
         void issue( const name& to, const asset& quantity, const string& memo );

         /**
          * The opposite for issue action, if all validations succeed,
          * it debits the statstable.supply amount.
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
          */
         [[eosio::action]]
         void transfer( const name&    from,
                        const name&    to,
                        const asset&   quantity,
                        const string&  memo );
         /**
          * Allows `ram_payer` to create an account `owner` with zero balance for
          * token `symbol` at the expense of `ram_payer`.
          *
          * @param owner - the account to be created,
          * @param symbol - the token to be payed with by `ram_payer`,
          * @param ram_payer - the account that supports the cost of this action.
          *
          * More information can be read [here](https://github.com/EOSIO/eosio.contracts/issues/62)
          * and [here](https://github.com/EOSIO/eosio.contracts/issues/61).
          */
         [[eosio::action]]
         void open( const name& owner, const symbol& symbol, const name& ram_payer );

         /**
          * This action is the opposite for open, it closes the account `owner`
          * for token `symbol`.
          *
          * @param owner - the owner account to execute the close action for,
          * @param symbol - the symbol of the token to execute the close action for.
          *
          * @pre The pair of owner plus symbol has to exist otherwise no action is executed,
          * @pre If the pair of owner plus symbol exists, the balance has to be zero.
          */
         [[eosio::action]]
         void close( const name& owner, const symbol& symbol );

         /**
          * This action freezes or unfreezes transaction processing
          * for token `symbol`.
          *
          * @param symbol - the symbol of the token to execute the freeze action for.
          * @param freeze - boolean, true = freeze, false = enable transfers.
          *
          * @pre The symbol has to exist otherwise no action is executed,
          * @pre Transaction must have the freeze_mgr authority 
          */
         [[eosio::action]]
         void freeze( const symbol& symbol, const bool& freeze );

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
         void resetram( const name& table, const string& scope, const uint32_t& limit );

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
         using issue_action = eosio::action_wrapper<"issue"_n, &token::issue>;
         using retire_action = eosio::action_wrapper<"retire"_n, &token::retire>;
         using transfer_action = eosio::action_wrapper<"transfer"_n, &token::transfer>;
         using open_action = eosio::action_wrapper<"open"_n, &token::open>;
         using close_action = eosio::action_wrapper<"close"_n, &token::close>;
         using freeze_action = eosio::action_wrapper<"freeze"_n, &token::freeze>;
         using resetram_action = eosio::action_wrapper<"resetram"_n, &token::resetram>;
      private:
         const name allowallacct = "allowallacct"_n;
         struct [[eosio::table]] account {
            asset    balance;

            uint64_t primary_key()const { return balance.symbol.code().raw(); }
         };

         struct [[eosio::table]] currency_stats {
            asset    supply;
            asset    max_supply;
            name     issuer;
            float  staking_ratio;
            name     membership_mgr;
            name     withdrawal_mgr;
            name     withdraw_to;
            name     freeze_mgr;
            bool     bearer_redeem;
            bool     config_locked;
            bool     transfers_frozen;

            uint64_t primary_key()const { return supply.symbol.code().raw(); }
         };

         typedef eosio::multi_index< "accounts"_n, account > accounts;
         typedef eosio::multi_index< "stat"_n, currency_stats > stats;

         void sub_balance( const name& owner, const asset& value );
         void add_balance( const name& owner, const asset& value, const name& ram_payer );
   };

}
