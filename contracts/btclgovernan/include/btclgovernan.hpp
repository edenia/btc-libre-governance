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
#include <eosio/singleton.hpp>
#include <eosio/asset.hpp>

using namespace std;
using namespace eosio;

CONTRACT btclgovernan : public contract
{
public:
   using contract::contract;

   /**
     * Set Params
     * 
     * This action set the params to be used on the voting process.
     *
     * @param vote_threshold
     * @param funding_account
     * @param voting_days
     * @param proposal_cost
     * @param approver
     * 
     * @return no return value.
     */
   ACTION setparams(uint8_t vote_threshold, name funding_account, uint8_t voting_days, asset proposal_cost, name approver);

   ACTION clear();

private:
   TABLE params
   {
      uint8_t vote_threshold;
      name funding_account;
      uint8_t voting_days;
      asset proposal_cost;
      name approver;
   };
   typedef singleton<name("params"), params> params_table;

};
