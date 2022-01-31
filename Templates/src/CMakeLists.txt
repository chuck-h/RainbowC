cmake_minimum_required(VERSION 3.10)
project(rainbow)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( rbtemplates rainbow_templates rainbow_templates.cpp )
target_include_directories( rainbow_templates PUBLIC ${CMAKE_SOURCE_DIR}/../include )
# target_ricardian_directory( rbtemplates ${CMAKE_SOURCE_DIR}/../ricardian )
# may cause cmake errors, see https://github.com/EOSIO/eosio.cdt/issues/1205
