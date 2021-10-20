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
   ACTION setparams(name funding_account, uint8_t vote_threshold, uint8_t voting_days, asset minimum_balance_to_create_proposals, asset proposal_cost, name approver);

   /**
     * crea
     * 
     * This action set the params to be used on the voting process.
     *
     * @param name
     * @param title
     * @param detail
     * @param amount
     * @param url
     * @param receiver
     * @param creator
     * 
     * @return no return value.
     */
   ACTION create(name creator, name receiver, name name, string title, string detail, asset amount, string url);

   bool is_valid_minimum_balance(name account);

   ACTION clear();

private:
   string EOSIO_TOKEN_CONTRACT = "eosio.token";
   
   string SUPPORTED_TOKEN = "XPR";

   enum proposal_status : uint8_t
   {
      DRAFT = 1,
      ACTIVE = 2,
      SUCCEEDED = 3,
      DEFEATED = 4,
      CANCELED = 5,
      COMPLETED = 6,
   };

   TABLE params
   {
      name funding_account;
      uint8_t vote_threshold;
      uint8_t voting_days;
      asset minimum_balance_to_create_proposals;
      asset proposal_cost;
      name approver;
   };
   typedef singleton<name("params"), params> params_table;

   TABLE proposals
   {
      name creator;
      name receiver;
      name name;
      string title;
      string detail;
      asset amount;
      string url;
      time_point_sec created_on;
      time_point_sec expires_on;
      uint8_t votes_for;
      uint8_t votes_against;
      uint8_t status;

      uint64_t primary_key() const { return name.value; }
   };
   typedef multi_index<name("proposals"), proposals> proposals_table;

   struct stats {
      asset supply;
      asset max_supply;
      name issuer;

      uint64_t primary_key() const { return supply.symbol.code().raw(); }
   };
   typedef multi_index<name("stat"), stats> stats_table;

   struct account {
      asset balance;

      uint64_t primary_key() const { return balance.symbol.code().raw(); }
   };
   typedef multi_index<name("accounts"), account> account_table;
};
