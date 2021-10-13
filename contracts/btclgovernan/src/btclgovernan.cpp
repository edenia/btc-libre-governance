/*
*
* @author  EOSCostaRica.io [https://eoscostarica.io]
*
* @section DESCRIPTION
*  Source file for the actions related with the btclgovernan contract
*
*  GitHub: https://github.com/eoscostarica/btc-libre-governance
*
*/

#include <btclgovernan.hpp>
#include <eosio/system.hpp>

ACTION btclgovernan::clear()
{
  require_auth(get_self());
}
