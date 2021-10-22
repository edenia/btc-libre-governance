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

ACTION btclgovernan::setparams(name funding_account, uint8_t vote_threshold, uint8_t voting_days, asset minimum_balance_to_create_proposals, asset proposal_cost, name approver)
{
  require_auth(get_self());

  check(minimum_balance_to_create_proposals.symbol.code().to_string() == SUPPORTED_TOKEN, "invalid minimum_balance_to_create_proposals symbol");
  check(proposal_cost.symbol.code().to_string() == SUPPORTED_TOKEN, "invalid proposal_cost symbol");

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

ACTION btclgovernan::create(name creator, name receiver, name name, string title, string detail, asset amount, string url)
{
  // ask permission of creator account
  require_auth(creator);

  // validate the token code
  check(amount.symbol.code().to_string() == SUPPORTED_TOKEN, "invalid amount symbol");

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
  _proposals.emplace(get_self(), [&](auto &ref)
  {
    ref.creator = creator;
    ref.receiver = receiver;
    ref.name = name;
    ref.title = title;
    ref.detail = detail;
    ref.amount = amount;
    ref.url = url;
    ref.created_on = current_time_point();
    ref.votes_for = 0;
    ref.votes_against = 0;
    ref.status = proposal_status::DRAFT;
  });
}

void btclgovernan::ontransfer(name from, name to, asset quantity, string memo)
{
  // skip transactions that are not for btclgovernan
  if (from == get_self() || to != get_self())
  {
    return;
  }

  if (memo.rfind(PAYMENT_TRANSFER, 0) == 0)
  {
    payment_handler(name(memo.substr(PAYMENT_TRANSFER.size())), quantity);
    return;
  }

  if (memo.rfind(DONATION_TRANSFER, 0) == 0)
  {
    return;
  }

  if (memo.rfind(FOUNDING_TRANSFER, 0) == 0)
  {
    return;
  }

  check(false, "invalid memo");
}

ACTION btclgovernan::clear()
{
  require_auth(get_self());
}

// helper functions

bool btclgovernan::is_valid_minimum_balance(name account)
{
  account_table _accounts(eosio::name(EOSIO_TOKEN_CONTRACT), account.value);
  params_table _params(get_self(), get_self().value);
  auto params_data = _params.get_or_create(get_self());
  bool valid = false;

  for (auto it = _accounts.begin(); it != _accounts.end(); it++)
  {
    if (it->balance.symbol.code().to_string() == SUPPORTED_TOKEN && it->balance.amount >= params_data.minimum_balance_to_create_proposals.amount)
    {
      valid = true;
      break;
    }
  }

  return valid;
}

void btclgovernan::payment_handler(name proposal, asset quantity)
{
  // validate the proposals exist with draft status
  proposals_table _proposals(get_self(), get_self().value);
  auto _proposal = _proposals.find(proposal.value);
  check(_proposal != _proposals.end(), "proposal not found");
  check(_proposal->status == proposal_status::DRAFT, "invalid proposal status");

  // get the params data
  params_table _params(get_self(), get_self().value);
  auto params_data = _params.get_or_create(get_self());

  // validte the quantity
  check(params_data.proposal_cost.symbol.code().to_string() == SUPPORTED_TOKEN, "invalid quantity symbol");
  check(quantity.amount == params_data.proposal_cost.amount, "invalid quantity amount");

  // change the proposal status to start the voting process
  _proposals.modify(_proposal, get_self(), [&](auto &ref)
  {
    ref.expires_on = current_time_point() + days(params_data.voting_days);
    ref.status = proposal_status::ACTIVE;
  });
}