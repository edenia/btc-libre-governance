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
#include <eosio/system.hpp>

ACTION btclgovernan::setparams(name funding_account, uint8_t vote_threshold, uint8_t voting_days, asset minimum_balance_to_create_proposals, asset proposal_cost, name approver) {
  require_auth(get_self());

  check(minimum_balance_to_create_proposals.symbol.code().to_string() == SUPPORTED_TOKEN, "invalid token code");
  check(proposal_cost.symbol.code().to_string() == SUPPORTED_TOKEN, "invalid token code");

  params_table _params(get_self(), get_self().value);
  auto data = _params.get_or_create(get_self());
  data.funding_account = funding_account;
  data.vote_threshold = vote_threshold;
  data.voting_days = voting_days;
  data.minimum_balance_to_create_proposals = minimum_balance_to_create_proposals;
  data.proposal_cost = proposal_cost;
  data.approver = approver;
  _params.set(data, get_self());
}

ACTION btclgovernan::create(name creator, name receiver, name name, string title, string detail, asset amount, string url) {
  // ask permission of creator account
  require_auth(creator);

  // validate the token code
  check(amount.symbol.code().to_string() == SUPPORTED_TOKEN, "invalid token code");

  // validate that there is no proposal with the same name
  proposals_table _proposals(get_self(), get_self().value);
  auto _proposal = _proposals.find(name.value);
  check(_proposal == _proposals.end(), "name already in use");

  // validate the minimum balance of the creator account
  check(is_valid_minimum_balance(creator), "not enough balance");
  

  // get the params data
  params_table _params(get_self(), get_self().value);
  auto params_data = _params.get_or_create(get_self());

  // save the proposal as a draft
  _proposals.emplace(get_self(), [ & ](auto & ref) {
    ref.creator = creator;
    ref.receiver = receiver;
    ref.name = name;
    ref.title = title;
    ref.detail = detail;
    ref.amount = amount;
    ref.url = url;
    ref.created_on = current_time_point();
    ref.expires_on = current_time_point() + days(params_data.voting_days);
    ref.votes_for = 0;
    ref.votes_against = 0;
    ref.status = proposal_status::DRAFT;
  });
}

ACTION btclgovernan::clear() {
  require_auth(get_self());
}

bool btclgovernan::is_valid_minimum_balance (name account) {
  account_table _accounts(eosio::name(EOSIO_TOKEN_CONTRACT), account.value);
  params_table _params(get_self(), get_self().value);
  auto params_data = _params.get_or_create(get_self());
  bool valid = false;
  
  for (auto it = _accounts.begin(); it != _accounts.end(); it++)
  {
    if (it->balance.symbol.code().to_string() == SUPPORTED_TOKEN && it->balance.amount >= params_data.minimum_balance_to_create_proposals.amount) {
      valid = true;
      break;
    }
  }

  return valid;
}
