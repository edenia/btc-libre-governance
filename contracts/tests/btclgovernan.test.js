/**
 * @file Unit test for btclgovernan program {@link https://github.com/eoscostarica/btc-libre-governance}.
 * @author EOSCostaRica <https://info@eoscostarica.io>
 * @version 1.1.0
 */
const assert = require('assert')

const utils = require('./utils')

const EOSIO_TOKEN_CONTRACT = 'eosio.token'
const SUPPORTED_TOKEN = 'XPR'
const MINIMUM_BALANCE_TO_CREATE_PROPOSALS = '1000.0000'
const PROPOSAL_COST = '1.0000'
const PROPOSAL_STATUS = {
  DRAFT: 1,
  ACTIVE: 2,
  SUCCEEDED: 3,
  DEFEATED: 4,
  CANCELED: 5,
  COMPLETED: 6
}

let eoslime
let eosioAccount
let eosioToken
let btclgovernan

const createProposal = async (creator, title = 'proposal') => {
  const receiver = await eoslime.Account.createRandom(eosioAccount)
  const name = await eoslime.utils.randomName()
  const detail = 'Lorem ipsum dolor sit amet, consectetur adipiscing elit.'
  const amount = `2.0000 ${SUPPORTED_TOKEN}`
  const url = 'http://localhost:3000/'

  await btclgovernan.contract.actions.create.broadcast(
    [creator.name, receiver.name, name, title, detail, amount, url],
    creator
  )

  return {
    creator,
    receiver,
    name,
    title,
    amount,
    url
  }
}

const proposalVote = async (proposal, voter, isFor) => {
  if (isFor) {
    await btclgovernan.contract.actions.votefor.broadcast(
      [voter.name, proposal.name],
      voter
    )
  } else {
    await btclgovernan.contract.actions.voteagainst.broadcast(
      [voter.name, proposal.name],
      voter
    )
  }
}

const sendTokens = async (to, quantity, memo = '') => {
  await eosioToken.contract.actions.transfer.broadcast(
    [eosioAccount.name, to, `${quantity} ${SUPPORTED_TOKEN}`, memo],
    eosioAccount
  )
}

describe('btclgovernan contract', function () {
  this.timeout(15000)

  before(async () => {
    eoslime = utils.eoslimeInit()
    eosioAccount = eoslime.Account.load(
      process.env.TESTNET_EOSIO_ACCOUNT,
      process.env.TESTNET_EOSIO_PRIVATE_KEY
    )
    eosioToken = await eoslime.load(
      EOSIO_TOKEN_CONTRACT,
      process.env.TESTNET_EOSIO_PRIVATE_KEY
    )
    btclgovernan = await eoslime.load(
      'btclgovernan',
      process.env.TESTNET_EOSIO_PRIVATE_KEY,
      eosioAccount
    )
  })

  it('set params', async () => {
    await btclgovernan.contract.actions.setparams.broadcast([
      btclgovernan.account.name,
      10,
      1,
      `${MINIMUM_BALANCE_TO_CREATE_PROPOSALS} ${SUPPORTED_TOKEN}`,
      `${PROPOSAL_COST} ${SUPPORTED_TOKEN}`,
      btclgovernan.account.name
    ])

    const rows = await btclgovernan.contract.tables.params.limit(1).find()

    assert.strictEqual(rows[0].approver, btclgovernan.account.name)
  })

  it('return an error when symbol is invalid ', async () => {
    try {
      await btclgovernan.contract.actions.setparams.broadcast([
        btclgovernan.account.name,
        10,
        1,
        `${MINIMUM_BALANCE_TO_CREATE_PROPOSALS} EOS`,
        `${MINIMUM_BALANCE_TO_CREATE_PROPOSALS} EOS`,
        btclgovernan.account.name
      ])
      throw new Error('N/A')
    } catch (error) {
      assert.strictEqual(
        error.message,
        'assertion failure with message: invalid minimum_balance_to_create_proposals symbol'
      )
    }
  })

  it('success on create proposal', async () => {
    const creator = await eoslime.Account.createRandom(eosioAccount)

    await sendTokens(creator.name, MINIMUM_BALANCE_TO_CREATE_PROPOSALS)

    const proposal = await createProposal(creator, 'success on create proposal')
    const rows = await btclgovernan.contract.tables.proposals
      .equal(proposal.name)
      .find()

    assert.strictEqual(rows[0].status, PROPOSAL_STATUS.DRAFT)
  })

  it('return an error when create proposal without enough balance', async () => {
    try {
      const creator = await eoslime.Account.createRandom(eosioAccount)

      await createProposal(
        creator,
        'return an error when create proposal without enough balance'
      )
      throw new Error('N/A')
    } catch (error) {
      assert.strictEqual(
        error.message,
        'assertion failure with message: not enough balance'
      )
    }
  })

  it('change the proposal to active status', async () => {
    const creator = await eoslime.Account.createRandom(eosioAccount)

    await sendTokens(creator.name, MINIMUM_BALANCE_TO_CREATE_PROPOSALS)

    const proposal = await createProposal(
      creator,
      'change the proposal to active status'
    )

    await sendTokens(
      btclgovernan.account.name,
      PROPOSAL_COST,
      `payment:${proposal.name}`
    )

    const rows = await btclgovernan.contract.tables.proposals
      .equal(proposal.name)
      .find()

    assert.strictEqual(rows[0].status, PROPOSAL_STATUS.ACTIVE)
  })

  it('return an error when proposal is invalid', async () => {
    try {
      await sendTokens(
        btclgovernan.account.name,
        MINIMUM_BALANCE_TO_CREATE_PROPOSALS,
        'payment:aaaabbbbcccc'
      )
      throw new Error('N/A')
    } catch (error) {
      assert.strictEqual(
        error.message,
        'assertion failure with message: proposal not found'
      )
    }
  })

  it('return an error when memo is invalid', async () => {
    try {
      await sendTokens(
        btclgovernan.account.name,
        MINIMUM_BALANCE_TO_CREATE_PROPOSALS,
        'spam transfer'
      )
      throw new Error('N/A')
    } catch (error) {
      assert.strictEqual(
        error.message,
        'assertion failure with message: invalid memo'
      )
    }
  })

  it('return an error when quantity is invalid', async () => {
    try {
      const creator = await eoslime.Account.createRandom(eosioAccount)

      await sendTokens(creator.name, MINIMUM_BALANCE_TO_CREATE_PROPOSALS)

      const proposal = await createProposal(
        creator,
        'return an error when quantity is invalid'
      )

      await sendTokens(
        btclgovernan.account.name,
        '9999.0000',
        `payment:${proposal.name}`
      )
      throw new Error('N/A')
    } catch (error) {
      assert.strictEqual(
        error.message,
        'assertion failure with message: invalid quantity amount'
      )
    }
  })

  it('success vote for proposal', async () => {
    const creator = await eoslime.Account.createRandom(eosioAccount)

    await sendTokens(creator.name, MINIMUM_BALANCE_TO_CREATE_PROPOSALS)

    const proposal = await createProposal(creator, 'success vote for proposal')

    await sendTokens(
      btclgovernan.account.name,
      PROPOSAL_COST,
      `payment:${proposal.name}`
    )

    const voter = await eoslime.Account.createRandom(eosioAccount)

    await sendTokens(voter.name, '1.0000')
    await proposalVote(proposal, voter, true)

    const rows = await btclgovernan.contract.tables.votes
      .scope(proposal.name)
      .equal(voter.name)
      .find()
    assert.strictEqual(rows[0].is_for, 1)
  })

  it('success vote against proposal', async () => {
    const creator = await eoslime.Account.createRandom(eosioAccount)

    await sendTokens(creator.name, MINIMUM_BALANCE_TO_CREATE_PROPOSALS)

    const proposal = await createProposal(
      creator,
      'success vote against proposal'
    )

    await sendTokens(
      btclgovernan.account.name,
      PROPOSAL_COST,
      `payment:${proposal.name}`
    )

    const voter = await eoslime.Account.createRandom(eosioAccount)

    await sendTokens(voter.name, '1.0000')
    await proposalVote(proposal, voter, false)

    const rows = await btclgovernan.contract.tables.votes
      .scope(proposal.name)
      .equal(voter.name)
      .find()
    assert.strictEqual(rows[0].is_for, 0)
  })

  it('return an error when vote with an invalid balance', async () => {
    try {
      const creator = await eoslime.Account.createRandom(eosioAccount)

      await sendTokens(creator.name, MINIMUM_BALANCE_TO_CREATE_PROPOSALS)

      const proposal = await createProposal(
        creator,
        'return an error when vote with an invalid balance'
      )

      await sendTokens(
        btclgovernan.account.name,
        PROPOSAL_COST,
        `payment:${proposal.name}`
      )

      const voter = await eoslime.Account.createRandom(eosioAccount)

      await proposalVote(proposal, voter, true)
      throw new Error('N/A')
    } catch (error) {
      assert.strictEqual(
        error.message,
        'assertion failure with message: not enough balance'
      )
    }
  })

  it('return an error when vote for invalid proposal', async () => {
    try {
      const creator = await eoslime.Account.createRandom(eosioAccount)

      await sendTokens(creator.name, MINIMUM_BALANCE_TO_CREATE_PROPOSALS)

      const proposal = await createProposal(
        creator,
        'return an error when vote for invalid proposal'
      )
      const voter = await eoslime.Account.createRandom(eosioAccount)

      await sendTokens(voter.name, '1.0000')
      await proposalVote(proposal, voter, true)
      throw new Error('N/A')
    } catch (error) {
      assert.strictEqual(
        error.message,
        'assertion failure with message: invalid proposal status'
      )
    }
  })

  it('return an error when count votes for a proposal that is still in voting', async () => {
    try {
      const creator = await eoslime.Account.createRandom(eosioAccount)

      await sendTokens(creator.name, '1000.0000')

      const proposal = await createProposal(
        creator,
        'return an error when count votes for a proposal that is still in voting'
      )

      await sendTokens(
        btclgovernan.account.name,
        '1.0000',
        `payment:${proposal.name}`
      )

      const voter = await eoslime.Account.createRandom(eosioAccount)

      await sendTokens(voter.name, '1.0000')
      await proposalVote(proposal, voter, true)
      await btclgovernan.contract.actions.countvotes.broadcast(
        [proposal.name],
        btclgovernan.account
      )
      throw new Error('N/A')
    } catch (error) {
      assert.strictEqual(
        error.message,
        'assertion failure with message: the voting period is not yet over'
      )
    }
  })
})
