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

ACTION btclgovernan::setparams(uint8_t vote_threshold, name funding_account, uint8_t voting_days, asset proposal_cost, name approver)
{
  require_auth(get_self());

  params_table _params(get_self(), get_self().value);
  auto data = _params.get_or_create(get_self());
  data.vote_threshold = vote_threshold;
  data.funding_account = funding_account;
  data.voting_days = voting_days;
  data.proposal_cost = proposal_cost;
  data.approver = approver;
  _params.set(data, get_self());
}

ACTION btclgovernan::clear()
{
  require_auth(get_self());
}
