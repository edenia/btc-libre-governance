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
    * Create
    *
    * This action create a new proposal.
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

   /**
    * On Transfer
    *
    * Handler for proposals payments and other token transfer
    *
    * @param from
    * @param to
    * @param quantity
    * @param memo
    *
    * @return no return value.
    */
   [[eosio::on_notify("*::transfer")]] void ontransfer(name from, name to, asset quantity, string memo);

   /**
    * Vote For
    *
    * This action vote for a proposal.
    *
    * @param proposal
    *
    * @return no return value.
    */
   ACTION votefor(name voter, name proposal);

   /**
    * Vote Against
    *
    * This action vote against a proposal.
    *
    * @param proposal
    *
    * @return no return value.
    */
   ACTION voteagainst(name voter, name proposal);

   /**
    * Count Votes
    *
    * This action count the votes for a proposal.
    *
    * @param proposal
    *
    * @return no return value.
    */
   ACTION countvotes(name proposal);

   /**
    * Approve
    *
    * This action give a final approval for a proposal.
    *
    * @param proposal
    *
    * @return no return value.
    */
   ACTION approve(name proposal);

   /**
    * Reject
    *
    * This action reject a proposal.
    *
    * @param proposal
    *
    * @return no return value.
    */
   ACTION reject(name proposal);

   asset get_account_balance(name account);

   void payment_handler(name proposal, asset quantity);

   void save_vote(name voter, name proposal, bool is_for);

private:
   string EOSIO_TOKEN_CONTRACT = "eosio.token";
   string SUPPORTED_TOKEN_SYMBOL = "XPR";
   uint8_t SUPPORTED_TOKEN_PRECISION = 4;

   string PAYMENT_TRANSFER = "payment:";
   string DONATION_TRANSFER = "donation";
   string FOUNDING_TRANSFER = "founding";

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
      eosio::name name;
      eosio::name creator;
      eosio::name receiver;
      string title;
      string detail;
      asset amount;
      string url;
      time_point_sec created_on;
      time_point_sec expires_on;
      int64_t votes_for;
      int64_t votes_against;
      uint8_t status;

      uint64_t primary_key() const { return name.value; }
      uint64_t by_created_on() const { return created_on.sec_since_epoch(); }
      uint64_t by_expires_on() const { return expires_on.sec_since_epoch(); }
   };
   typedef multi_index<
      name("proposals"),
      proposals,
      indexed_by<name("createdon"), const_mem_fun<proposals, uint64_t, &proposals::by_created_on>>,
      indexed_by<name("expireson"), const_mem_fun<proposals, uint64_t, &proposals::by_expires_on>>
   > proposals_table;

   TABLE votes
   {
      name voter;
      bool is_for;
      int64_t quantity;

      uint64_t primary_key() const { return voter.value; }
   };
   typedef multi_index<name("votes"), votes> votes_table;

   struct stats
   {
      asset supply;
      asset max_supply;
      name issuer;

      uint64_t primary_key() const { return supply.symbol.code().raw(); }
   };
   typedef multi_index<name("stat"), stats> stats_table;

   struct account
   {
      asset balance;

      uint64_t primary_key() const { return balance.symbol.code().raw(); }
   };
   typedef multi_index<name("accounts"), account> account_table;
};
