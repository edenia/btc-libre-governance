/*
*
* @author  EOSCostaRica.io [https://eoscostarica.io]
*
* @section DESCRIPTION
* Header file for the declaration of all functions related with the btclgovernan contract
*
* GitHub: https://github.com/eoscostarica/btc-libre-governance
*
*/
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

using namespace std;
using namespace eosio;

CONTRACT btclgovernan : public contract
{
public:
   using contract::contract;

   ACTION clear();

private:
};
