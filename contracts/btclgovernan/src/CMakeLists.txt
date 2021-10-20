cmake_minimum_required(VERSION 3.5)
project(btclgovernan)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( btclgovernan btclgovernan btclgovernan.cpp )
target_include_directories( btclgovernan PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( btclgovernan '${CMAKE_SOURCE_DIR}/../ricardian/' )