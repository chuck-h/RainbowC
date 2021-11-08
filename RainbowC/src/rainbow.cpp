#include <rainbow.hpp>
#include <../capi/eosio/action.h>

namespace eosio {

void token::create( const name&   issuer,
                    const asset&  maximum_supply,
                    const name&   membership_mgr,
                    const name&   withdrawal_mgr,
                    const name&   withdraw_to,
                    const name&   freeze_mgr,
                    const bool&   bearer_redeem,
                    const string& config_locked_until_string)
{
    require_auth( issuer );
    auto sym = maximum_supply.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    check( maximum_supply.is_valid(), "invalid supply");
    check( maximum_supply.amount > 0, "max-supply must be positive");
    check( is_account( membership_mgr ) || membership_mgr == allowallacct,
        "membership_mgr account does not exist");
    check( is_account( withdrawal_mgr ), "withdrawal_mgr account does not exist");
    check( is_account( withdraw_to ), "withdraw_to account does not exist");
    check( is_account( freeze_mgr ), "freeze_mgr account does not exist");
    time_point config_locked_until = current_time_point();
    if( config_locked_until_string != "" ) {
       config_locked_until = time_point::from_iso_string( config_locked_until_string );
       auto days_from_now = (config_locked_until.time_since_epoch() -
                             current_time_point().time_since_epoch()).count()/days(1).count();
       check( days_from_now < 100*365 && days_from_now > -10*365, "config lock date out of range" );
    }
    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    if( existing != statstable.end()) {
       // token exists
       const auto& st = *existing;
       configs configtable( get_self(), sym.code().raw() );
       const auto& cf = *configtable.begin();
       check( cf.config_locked_until.time_since_epoch() < current_time_point().time_since_epoch(),
              "token reconfiguration is locked" );
       check( st.issuer == issuer, "mismatched issuer account" );
       if( st.supply.amount != 0 ) {
          check( sym == st.supply.symbol,
                 "cannot change symbol precision with outstanding supply" );
          //TBD: could support precision change by walking accounts table
          check( maximum_supply.amount >= st.supply.amount,
                 "cannot reduce maximum below outstanding supply" );
       }
       statstable.modify (st, issuer, [&]( auto& s ) {
          s.supply.symbol = maximum_supply.symbol;
          s.max_supply    = maximum_supply;
          s.issuer        = issuer;
       });
       configtable.modify (cf, issuer, [&]( auto& s ) {
          s.membership_mgr = membership_mgr;
          s.withdrawal_mgr = withdrawal_mgr;
          s.withdraw_to   = withdraw_to;
          s.freeze_mgr    = freeze_mgr;
          s.bearer_redeem = bearer_redeem;
          s.config_locked_until = config_locked_until;
       });
    return;
    }
    // new token
    require_auth2( get_self().value, create_token_permission.value );
    statstable.emplace( issuer, [&]( auto& s ) {
       s.supply.symbol = maximum_supply.symbol;
       s.max_supply    = maximum_supply;
       s.issuer        = issuer;
    });
    configs configtable( get_self(), sym.code().raw() );
    configtable.emplace( issuer, [&]( auto& s ) {
       s.membership_mgr = membership_mgr;
       s.withdrawal_mgr = withdrawal_mgr;
       s.withdraw_to   = withdraw_to;
       s.freeze_mgr    = freeze_mgr;
       s.bearer_redeem = bearer_redeem;
       s.config_locked_until = config_locked_until;
       s.transfers_frozen = false;

    });
}

void token::setstake( const name&   issuer,
                      const asset&  token_bucket,
                      const asset&  stake_per_bucket,
                      const name&   stake_token_contract,
                      const name&   stake_to,
                      const string& memo)
{
    require_auth( issuer );
    check( memo.size() <= 256, "memo has more than 256 bytes" );
    auto stake_sym = stake_per_bucket.symbol;
    uint128_t stake_token = (uint128_t)stake_sym.raw()<<64 | stake_token_contract.value;
    check( stake_sym.is_valid(), "invalid stake symbol name" );
    check( stake_per_bucket.is_valid(), "invalid stake");
    check( stake_per_bucket.amount >= 0, "stake per token must be non-negative");
    check( is_account( stake_token_contract ), "stake token contract account does not exist");
    accounts accountstable( stake_token_contract, issuer.value );
    const auto stake_bal = accountstable.find( stake_sym.code().raw() );
    check( stake_bal != accountstable.end(), "issuer must have a stake token balance");
    check( stake_bal->balance.symbol == stake_sym, "mismatched stake token precision" );
    if( stake_to != deletestakeacct ) {
       check( is_account( stake_to ), "stake_to account does not exist");
    }
    check( token_bucket.amount > 0, "token bucket must be > 0" );
    auto sym_code_raw = token_bucket.symbol.code().raw();
    stats statstable( get_self(), sym_code_raw );
    const auto& st = statstable.get( sym_code_raw, "token with symbol does not exist" );
    configs configtable( get_self(), sym_code_raw );
    const auto& cf = *configtable.begin();
    check( cf.config_locked_until.time_since_epoch() < current_time_point().time_since_epoch(),
           "token reconfiguration is locked" );
    check( st.issuer == issuer, "mismatched issuer account" );
    stakes stakestable( get_self(), sym_code_raw );
    auto stake_token_index = stakestable.get_index<"staketoken"_n>();
    auto existing = stake_token_index.find( stake_token );
    if( existing != stake_token_index.end()) {
       // stake token exists in stakes table
       const auto& sk = *existing;
       bool restaking = token_bucket != sk.token_bucket ||
                        stake_per_bucket != sk.stake_per_bucket ||
                        stake_to != sk.stake_to;
       bool destaking = stake_to == sk.stake_to &&
                        stake_per_bucket.amount == 0;
       if( st.supply.amount != 0 ) {
          if( destaking ) {
             unstake_one( st, sk, st.issuer, st.supply.amount );
          } else if ( restaking ) {
             check( sk.stake_per_bucket.amount == 0, "must destake before restaking");
             if( stake_to == deletestakeacct ) {
                stakestable.erase( sk );
             }
          }
       }
       stakestable.modify (sk, issuer, [&]( auto& s ) {
          s.token_bucket = token_bucket;
          s.stake_per_bucket = stake_per_bucket;
          s.stake_token_contract = stake_token_contract;
          s.stake_to = stake_to;
       });
       if( restaking ) {
          stake_one( st, sk, st.supply.amount );
       }
       return;
    }
    // new stake token
    int existing_stake_count = std::distance(stakestable.cbegin(),stakestable.cend());
    check( existing_stake_count <= max_stake_count, "stake count exceeded" );
    check( stake_to != deletestakeacct, "invalid stake_to account" );
    const auto& sk = *stakestable.emplace( issuer, [&]( auto& s ) {
       s.index                = stakestable.available_primary_key();
       s.token_bucket         = token_bucket;
       s.stake_per_bucket      = stake_per_bucket;
       s.stake_token_contract = stake_token_contract;
       s.stake_to             = stake_to;
    });
    if( st.supply.amount != 0 ) {
       stake_one( st, sk, st.supply.amount );
    }

}


void token::issue( const asset& quantity, const string& memo )
{
    auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
    const auto& st = *existing;
    require_auth( st.issuer );
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must issue positive quantity" );

    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    check( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.supply += quantity;
    });

    stake_all( st, quantity.amount );
    add_balance( st.issuer, quantity, st.issuer );
}

void token::stake_one( const currency_stats st, const stake_stats sk, const uint64_t amount ) {
    if( sk.stake_per_bucket.amount > 0 ) {
       asset stake_quantity = sk.stake_per_bucket;
       stake_quantity.amount = (int64_t)((int128_t)amount*sk.stake_per_bucket.amount/sk.token_bucket.amount);
       // TBD: are there potential exploits based on rounding inaccuracy?
       action(
          permission_level{st.issuer,"active"_n},
          sk.stake_token_contract,
          "transfer"_n,
          std::make_tuple(st.issuer,
                          sk.stake_to,
                          stake_quantity,
                          std::string("rainbow stake"))
       ).send();
    }
}

void token::stake_all( const currency_stats st, const uint64_t amount ) {
    stakes stakestable( get_self(), st.supply.symbol.code().raw() );
    for( auto itr = stakestable.begin(); itr != stakestable.end(); itr++ ) {
       stake_one( st, *itr, amount );
    }
}

void token::unstake_one( const currency_stats st, const stake_stats sk, const name& owner, const uint64_t amount ) {
    if( sk.stake_per_bucket.amount > 0 ) {
       asset stake_quantity = sk.stake_per_bucket;
       stake_quantity.amount = (int64_t)((int128_t)amount*sk.stake_per_bucket.amount/sk.token_bucket.amount);
       // TBD: are there potential exploits based on rounding inaccuracy?
       action(
          permission_level{sk.stake_to,"active"_n},
          sk.stake_token_contract,
          "transfer"_n,
          std::make_tuple(sk.stake_to,
                          owner,
                          stake_quantity,
                          std::string("rainbow unstake"))
       ).send();
    }
}
void token::unstake_all( const currency_stats st, const name& owner, const uint64_t amount ) {
    stakes stakestable( get_self(), st.supply.symbol.code().raw() );
    for( auto itr = stakestable.begin(); itr != stakestable.end(); itr++ ) {
       unstake_one( st, *itr, owner, amount );
    }
}

void token::retire( const name& owner, const asset& quantity, const string& memo )
{
    auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    stats statstable( get_self(), sym.code().raw() );
    const auto& st = statstable.get( sym.code().raw(), "token with symbol does not exist" );
    configs configtable( get_self(), sym.code().raw() );
    const auto& cf = *configtable.begin();
    if( cf.bearer_redeem ) {
       check( !cf.transfers_frozen, "transfers are frozen");
    } else {
       check( owner == st.issuer, "bearer redeem is disabled");
    }
    require_auth( owner );
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must retire positive quantity" );

    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );

    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.supply -= quantity;
    });

    sub_balance( owner, quantity );
    unstake_all( st, owner, quantity.amount );
}

void token::transfer( const name&    from,
                      const name&    to,
                      const asset&   quantity,
                      const string&  memo )
{
    check( from != to, "cannot transfer to self" );
    check( is_account( from ), "from account does not exist");
    check( is_account( to ), "to account does not exist");
    auto sym_code_raw = quantity.symbol.code().raw();
    stats statstable( get_self(), sym_code_raw );
    const auto& st = statstable.get( sym_code_raw );
    configs configtable( get_self(), sym_code_raw );
    const auto& cf = *configtable.begin();

    if( cf.membership_mgr != allowallacct ) {
       accounts to_acnts( get_self(), to.value );
       auto to = to_acnts.find( sym_code_raw );
       check( to != to_acnts.end(), "to account must have membership");
    }

    require_recipient( from );
    require_recipient( to );

    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must transfer positive quantity" );
    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    bool withdrawing = has_auth( cf.withdrawal_mgr ) && to == cf.withdraw_to;
    if (!withdrawing ) {
       require_auth( from );
       if( from != st.issuer ) {
          check( !cf.transfers_frozen, "transfers are frozen");
       }
       // TBD: implement token.seeds check_limit_transactions(from) ?
    }

    auto payer = has_auth( to ) ? to : from;

    sub_balance( from, quantity );
    add_balance( to, quantity, payer );
}

void token::sub_balance( const name& owner, const asset& value ) {
   accounts from_acnts( get_self(), owner.value );

   const auto& from = from_acnts.get( value.symbol.code().raw(), "no balance object found" );
   check( from.balance.amount >= value.amount, "overdrawn balance" );

   from_acnts.modify( from, same_payer, [&]( auto& a ) {
         a.balance -= value;
      });
}

void token::add_balance( const name& owner, const asset& value, const name& ram_payer )
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

void token::open( const name& owner, const symbol_code& symbolcode, const name& ram_payer )
{
   require_auth( ram_payer );

   check( is_account( owner ), "owner account does not exist" );

   auto sym_code_raw = symbolcode.raw();
   stats statstable( get_self(), sym_code_raw );
   const auto& st = statstable.get( sym_code_raw, "symbol does not exist" );
   configs configtable( get_self(), sym_code_raw );
   const auto& cf = *configtable.begin();
   if( cf.membership_mgr != allowallacct) {
      require_auth( cf.membership_mgr );
   }
   accounts acnts( get_self(), owner.value );
   auto it = acnts.find( sym_code_raw );
   if( it == acnts.end() ) {
      acnts.emplace( ram_payer, [&]( auto& a ){
        a.balance = asset{0, st.supply.symbol};
      });
   }
}

void token::close( const name& owner, const symbol_code& symbolcode )
{
   auto sym_code_raw = symbolcode.raw();
   stats statstable( get_self(), sym_code_raw );
   const auto& st = statstable.get( sym_code_raw, "symbol does not exist" );
   configs configtable( get_self(), sym_code_raw );
   const auto& cf = *configtable.begin();
   if( cf.membership_mgr == allowallacct || !has_auth( cf.membership_mgr ) ) {
      require_auth( owner );
   }
   accounts acnts( get_self(), owner.value );
   auto it = acnts.find( sym_code_raw );
   check( it != acnts.end(), "Balance row already deleted or never existed. Action won't have any effect." );
   check( it->balance.amount == 0, "Cannot close because the balance is not zero." );
   acnts.erase( it );
}

void token::freeze( const symbol_code& symbolcode, const bool& freeze, const string& memo )
{
   auto sym_code_raw = symbolcode.raw();
   stats statstable( get_self(), sym_code_raw );
   const auto& st = statstable.get( sym_code_raw, "symbol does not exist" );
   configs configtable( get_self(), sym_code_raw );
   const auto& cf = *configtable.begin();
   check( memo.size() <= 256, "memo has more than 256 bytes" );
   require_auth( cf.freeze_mgr );
   configtable.modify (cf, same_payer, [&]( auto& s ) {
      s.transfers_frozen = freeze;
   });
}

void token::resetram( const name& table, const string& scope, const uint32_t& limit )
{
   require_auth( get_self() );
   uint64_t scope_raw;
   check( !scope.empty(), "scope string is empty" );
   if( scope.length() <= 7 ) {
      symbol_code code(scope);
      check( code.is_valid(), "invalid symbol code" );
      scope_raw = code.raw();
   } else {
      name n(scope);
      scope_raw = n.value;
   }
   uint32_t counter = 0;
   if( table == "stat"_n ) {
      stats statstable( get_self(), scope_raw );
      for( auto itr = statstable.begin(); itr != statstable.end() && counter<limit; counter++ ) {
         itr = statstable.erase(itr);
      }
   } else if( table == "configs"_n ) {
      configs configtable( get_self(), scope_raw );
      for( auto itr = configtable.begin(); itr != configtable.end() && counter<limit; counter++ ) {
         itr = configtable.erase(itr);
      }
   } else if( table == "accounts"_n ) {
      accounts acnts( get_self(), scope_raw );
      for( auto itr = acnts.begin(); itr != acnts.end() && counter<limit; counter++ ) {
         itr = acnts.erase(itr);
      }
   } else if( table == "stakes"_n ) {
      stakes stakestable( get_self(), scope_raw );
      for( auto itr = stakestable.begin(); itr != stakestable.end() && counter<limit; counter++ ) {
         itr = stakestable.erase(itr);
      }
   } else {
      check( 0, "table name should be 'stat', 'configs', 'accounts', or 'stakes'" );
   }      
}


} /// namespace eosio
