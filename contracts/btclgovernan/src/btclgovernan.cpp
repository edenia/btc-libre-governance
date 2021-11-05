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

ACTION btclgovernan::setparams( name    funding_account,
                                uint8_t vote_threshold,
                                uint8_t voting_days,
                                asset   minimum_balance_to_create_proposals,
                                asset   proposal_cost,
                                name    approver )
{
    require_auth( get_self() );

    check( minimum_balance_to_create_proposals.symbol.code() == SUPPORTED_TOKEN_SYMBOL.code(),
           "invalid minimum_balance_to_create_proposals symbol" );
    check( proposal_cost.symbol.code() == SUPPORTED_TOKEN_SYMBOL.code(), "invalid proposal_cost symbol" );

    params_table _params( get_self(), get_self().value );

    auto data = _params.get_or_create( get_self() );
    data.funding_account = funding_account;
    data.vote_threshold = vote_threshold;
    data.voting_days = voting_days;
    data.minimum_balance_to_create_proposals = minimum_balance_to_create_proposals;
    data.proposal_cost = proposal_cost;
    data.approver = approver;
    _params.set( data, get_self() );
}

ACTION btclgovernan::create( name   creator,
                             name   receiver,
                             name   name,
                             string title,
                             string detail,
                             asset  amount,
                             string url )
{
    // ask permission of creator account
    require_auth( creator );

    // validate the token code
    check( amount.symbol.code() == SUPPORTED_TOKEN_SYMBOL.code(), "invalid amount symbol" );

    // validate that there is no proposal with the same name
    proposals_table _proposals( get_self(), get_self().value );
    auto            _proposal = _proposals.find( name.value );
    check( _proposal == _proposals.end(), "name already in use" );

    // get the params data
    params_table _params( get_self(), get_self().value );
    auto         params_data = _params.get_or_create( get_self() );

    // validate the minimum balance of the creator account
    asset balance = get_account_balance( creator );
    check( balance.amount >= params_data.minimum_balance_to_create_proposals.amount, "not enough balance" );

    // save the proposal as a draft
    _proposals.emplace( get_self(), [&]( auto &ref ) {
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
    } );
}

void btclgovernan::ontransfer( name from, name to, asset quantity, string memo )
{
    // skip transactions that are not for btclgovernan
    if ( from == get_self() || to != get_self() )
    {
        return;
    }

    if ( memo.rfind( PAYMENT_TRANSFER, 0 ) == 0 )
    {
        payment_handler( name( memo.substr( PAYMENT_TRANSFER.size() ) ), quantity );
        return;
    }

    check( memo.rfind( DONATION_TRANSFER, 0 ) == 0 || memo.rfind( FOUNDING_TRANSFER, 0 ) == 0, "invalid memo" );
}

void btclgovernan::votefor( name voter, name proposal )
{
    // ask permission of voter account
    require_auth( voter );

    // save the vote
    save_vote( voter, proposal, true );
}

void btclgovernan::voteagainst( name voter, name proposal )
{
    // ask permission of voter account
    require_auth( voter );

    // save the vote
    save_vote( voter, proposal, false );
}

void btclgovernan::countvotes( name proposal )
{
    // get the params data
    params_table _params( get_self(), get_self().value );
    auto         params_data = _params.get_or_create( get_self() );

    // ask permission of approver account
    require_auth( params_data.approver );

    // validate the proposals exist with active status
    proposals_table _proposals( get_self(), get_self().value );
    auto            _proposal = _proposals.find( proposal.value );
    check( _proposal != _proposals.end(), "proposal not found" );
    check( _proposal->status == proposal_status::ACTIVE, "invalid proposal status" );

    // validate if the proposal is ready to count votes
    check( _proposal->expires_on < current_time_point(), "the voting period is not yet over" );

    votes_table _votes( get_self(), proposal.value );
    int64_t     votes_for = 0;
    int64_t     votes_against = 0;

    // count the votes based on each voter balance
    for ( auto it = _votes.begin(); it != _votes.end(); it++ )
    {
        // get the voter balance
        asset balance = get_account_balance( it->voter );

        // increase the total votes
        if ( it->is_for )
        {
            votes_for += balance.amount;
        }
        else
        {
            votes_against += balance.amount;
        }

        // save the vote
        _votes.modify( it, get_self(), [&]( auto &ref ) { ref.quantity = balance.amount; } );
    }

    // get the current supply
    stats_table _stats( EOSIO_TOKEN_CONTRACT, SUPPORTED_TOKEN_SYMBOL.code().raw() );
    auto        _stat = _stats.begin();
    check( _stat != _stats.end(), "unable to read current supply" );

    // validate rules for a succeeds proposal
    float achieved_vote_threshold = (float)( votes_for ) / (float)_stat->supply.amount * 100;
    bool  has_valid_vote_threshold = achieved_vote_threshold >= params_data.vote_threshold;
    bool  has_majority = votes_for > votes_against;

    // save the result
    _proposals.modify( _proposal, get_self(), [&]( auto &ref ) {
        ref.votes_for = votes_for;
        ref.votes_against = votes_against;
        ref.status = has_majority && has_valid_vote_threshold ? proposal_status::SUCCEEDED : proposal_status::DEFEATED;
    } );
}

void btclgovernan::checkvotes( name proposal )
{
    // validate the proposals exist with active status
    proposals_table _proposals( get_self(), get_self().value );
    auto            _proposal = _proposals.find( proposal.value );
    check( _proposal != _proposals.end(), "proposal not found" );
    check( _proposal->status == proposal_status::ACTIVE, "invalid proposal status" );

    votes_table _votes( get_self(), proposal.value );
    int64_t     votes_for = 0;
    int64_t     votes_against = 0;

    // count the votes based on each voter balance
    for ( auto it = _votes.begin(); it != _votes.end(); it++ )
    {
        // get the voter balance
        asset balance = get_account_balance( it->voter );

        // increase the total votes
        if ( it->is_for )
        {
            votes_for += balance.amount;
        }
        else
        {
            votes_against += balance.amount;
        }

        // save the vote
        _votes.modify( it, get_self(), [&]( auto &ref ) { ref.quantity = balance.amount; } );
    }

    // save the result
    _proposals.modify( _proposal, get_self(), [&]( auto &ref ) {
        ref.votes_for = votes_for;
        ref.votes_against = votes_against;
    } );
}

void btclgovernan::approve( name proposal )
{
    // get the params data
    params_table _params( get_self(), get_self().value );
    auto         params_data = _params.get_or_create( get_self() );

    // ask permission of approver account
    require_auth( params_data.approver );

    // validate the proposals exist with active status
    proposals_table _proposals( get_self(), get_self().value );
    auto            _proposal = _proposals.find( proposal.value );
    check( _proposal != _proposals.end(), "proposal not found" );
    check( _proposal->status == proposal_status::SUCCEEDED, "invalid proposal status" );

    action( permission_level{ get_self(), name( "active" ) },
            EOSIO_TOKEN_CONTRACT,
            name( "transfer" ),
            std::make_tuple( params_data.funding_account,
                             _proposal->receiver,
                             _proposal->amount,
                             "funding for " + _proposal->title ) )
        .send();

    // save the new status
    _proposals.modify( _proposal, get_self(), [&]( auto &ref ) { ref.status = proposal_status::COMPLETED; } );
}

void btclgovernan::reject( name proposal )
{
    // get the params data
    params_table _params( get_self(), get_self().value );
    auto         params_data = _params.get_or_create( get_self() );

    // ask permission of approver account
    require_auth( params_data.approver );

    // validate the proposals exist with active status
    proposals_table _proposals( get_self(), get_self().value );
    auto            _proposal = _proposals.find( proposal.value );
    check( _proposal != _proposals.end(), "proposal not found" );
    check( _proposal->status == proposal_status::SUCCEEDED, "invalid proposal status" );

    // save the new status
    _proposals.modify( _proposal, get_self(), [&]( auto &ref ) { ref.status = proposal_status::CANCELED; } );
}

// helper functions

asset btclgovernan::get_account_balance( name account )
{
    account_table _accounts( EOSIO_TOKEN_CONTRACT, account.value );
    asset         result = asset( 0, SUPPORTED_TOKEN_SYMBOL );

    for ( auto it = _accounts.begin(); it != _accounts.end(); it++ )
    {
        if ( it->balance.symbol.code() == SUPPORTED_TOKEN_SYMBOL.code() && it->balance.amount > 0 )
        {
            result = it->balance;
            break;
        }
    }

    return result;
}

void btclgovernan::payment_handler( name proposal, asset quantity )
{
    // validate the proposals exist with draft status
    proposals_table _proposals( get_self(), get_self().value );
    auto            _proposal = _proposals.find( proposal.value );
    check( _proposal != _proposals.end(), "proposal not found" );
    check( _proposal->status == proposal_status::DRAFT, "invalid proposal status" );

    // get the params data
    params_table _params( get_self(), get_self().value );
    auto         params_data = _params.get_or_create( get_self() );

    // validate the quantity
    check( params_data.proposal_cost.symbol.code() == SUPPORTED_TOKEN_SYMBOL.code(), "invalid quantity symbol" );
    check( quantity.amount == params_data.proposal_cost.amount, "invalid quantity amount" );

    // change the proposal status to start the voting process
    _proposals.modify( _proposal, get_self(), [&]( auto &ref ) {
        ref.expires_on = current_time_point() + days( params_data.voting_days );
        ref.status = proposal_status::ACTIVE;
    } );
}

void btclgovernan::save_vote( name voter, name proposal, bool is_for )
{
    // validate the minimum balance of the voter account
    asset balance = get_account_balance( voter );
    check( balance.amount > 0, "not enough balance" );

    // validate the proposals exist with active status
    proposals_table _proposals( get_self(), get_self().value );
    auto            _proposal = _proposals.find( proposal.value );
    check( _proposal != _proposals.end(), "proposal not found" );
    check( _proposal->status == proposal_status::ACTIVE, "invalid proposal status" );

    // validate if the proposal is still available to receive votes
    check( _proposal->expires_on >= current_time_point(), "the voting period is over" );

    // validate if voter already voted for the proposal
    votes_table _votes( get_self(), proposal.value );
    auto        _vote = _votes.find( voter.value );
    check( _vote == _votes.end(), "already voted" );

    // save the vote
    _votes.emplace( get_self(), [&]( auto &ref ) {
        ref.voter = voter;
        ref.is_for = is_for;
        ref.quantity = balance.amount;
    } );
}
