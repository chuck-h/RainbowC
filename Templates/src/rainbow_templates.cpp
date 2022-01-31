#include <rainbow_templates.hpp>
#include <../capi/eosio/action.h>

const std::vector<string> rbtemplates::tablenames { "stat", "configs", "displays", "stakes", "all"};

void rbtemplates::importer( const name& caller,
                            const name& contract,
                            const symbol_code& symbolcode,
                            const string& table )
{
    require_auth(caller);
    check( symbolcode.is_valid(), "invalid symbol code" );
    auto sym_code_raw = symbolcode.raw();
    check( std::find(tablenames.begin(), tablenames.end(), table) != tablenames.end(), "invalid table name");
    check( is_account( contract ), "contract account does not exist");
    if( table == "stat" || table == "all" ) {
       stats tmpl_statstable( get_self(), sym_code_raw );
       auto existing = tmpl_statstable.find( sym_code_raw );
       check( existing == tmpl_statstable.end(), "cannot overwrite template" );
       stats statstable( contract, sym_code_raw );
       const auto& st = statstable.get( sym_code_raw, "token not in contract" );
       tmpl_statstable.emplace( caller, [&]( auto& s ) {
          s.supply.symbol = st.supply.symbol;
          s.max_supply    = st.max_supply;
          s.issuer        = st.issuer;
       });
    }
    if( table == "configs" || table == "all" ) {
       configs tmpl_configtable( get_self(), sym_code_raw );
       check( !tmpl_configtable.exists(), "cannot overwrite template" );
       configs configtable( contract, sym_code_raw );
       const auto& st = configtable.get();
       currency_config new_config{
          .membership_mgr = st.membership_mgr,
          .withdrawal_mgr = st.withdrawal_mgr,
          .withdraw_to   = st.withdraw_to,
          .freeze_mgr    = st.freeze_mgr,
          .redeem_locked_until = st.redeem_locked_until,
          .config_locked_until = st.config_locked_until,
          .transfers_frozen = st.transfers_frozen,
          .approved      = st.approved,
          .cred_limit = st.cred_limit,
          .positive_limit = st.positive_limit
       };
    tmpl_configtable.set( new_config, caller );
    }
    if( table == "displays" || table == "all" ) {
       displays tmpl_displaytable( get_self(), sym_code_raw );
       check( !tmpl_displaytable.exists(), "cannot overwrite template" );
       displays displaytable( contract, sym_code_raw );
       const auto& st = displaytable.get();
       currency_display new_display{
          .json_meta = st.json_meta
       };
    tmpl_displaytable.set( new_display, caller );
    }
    if( table == "stakes" || table == "all" ) {
       stakes tmpl_stakestable( get_self(), sym_code_raw );
       auto existing = tmpl_stakestable.find( sym_code_raw );
       check( existing == tmpl_stakestable.end(), "cannot overwrite template" );
       while( existing != tmpl_stakestable.end() ) {
          tmpl_stakestable.emplace( caller, [&]( auto& s ) {
             s.index                = tmpl_stakestable.available_primary_key();
             s.token_bucket         = existing->token_bucket;
             s.stake_per_bucket     = existing->stake_per_bucket;
             s.stake_token_contract = existing->stake_token_contract;
             s.stake_to             = existing->stake_to;
             s.proportional         = existing->proportional;
          });
          existing++;
       }
    }
}

void rbtemplates::erase(const symbol_code& symbolcode, const string& table ) {
    
   require_auth(get_self());
   check( symbolcode.is_valid(), "invalid symbol code" );
   auto sym_code_raw = symbolcode.raw();
   check( std::find(tablenames.begin(), tablenames.end(), table) != tablenames.end(), "invalid table name");
   if( table == "stat" || table == "all" ) {
       stats tbl(get_self(),sym_code_raw);
       auto itr = tbl.begin();
       while (itr != tbl.end()) {
         itr = tbl.erase(itr);
       }
   }
   if( table == "configs" || table == "all" ) {
       configs tbl(get_self(),sym_code_raw);
       tbl.remove();
   }
   if( table == "displays" || table == "all" ) {
       displays tbl(get_self(),sym_code_raw);
       tbl.remove();
   }
   if( table == "stakes" || table == "all" ) {
       stakes tbl(get_self(),sym_code_raw);
       auto itr = tbl.begin();
       while (itr != tbl.end()) {
         itr = tbl.erase(itr);
       }
   }
}
     



