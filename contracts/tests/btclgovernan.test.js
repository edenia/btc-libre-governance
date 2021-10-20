/**
 * @file Unit test for btclgovernan program {@link https://github.com/eoscostarica/btc-libre-governance}.
 * @author EOSCostaRica <https://info@eoscostarica.io>
 * @version 1.1.0
 */
const assert = require('assert')

const utils = require('./utils')

const EOSIO_TOKEN_CONTRACT = 'eosio.token'
const SUPPORTED_TOKEN = 'XPR'

let eoslime
let eosioAccount
let eosioToken
let btclgovernan

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

  it('should set params', async () => {
    const fundingAccount = btclgovernan.account.name
    const voteThreshold = 10
    const votingDays = 1
    const minimumBalanceToCreateProposals = `1000.0000 ${SUPPORTED_TOKEN}`
    const proposalCost = `1.0000 ${SUPPORTED_TOKEN}`
    const approver = btclgovernan.account.name
    const response = await btclgovernan.contract.actions.setparams.broadcast([
      fundingAccount,
      voteThreshold,
      votingDays,
      minimumBalanceToCreateProposals,
      proposalCost,
      approver
    ])
    const rows = await btclgovernan.contract.tables.params.limit(1).find()

    assert.strictEqual(rows[0].approver, btclgovernan.account.name)
    console.log(`    👍 setparams txid ${response.transaction_id}`)
  })

  it('should return error on invalid token code', async () => {
    const fundingAccount = btclgovernan.account.name
    const voteThreshold = 10
    const votingDays = 1
    const minimumBalanceToCreateProposals = '1.0000 EOS'
    const proposalCost = '1.0000 EOS'
    const approver = btclgovernan.account.name

    try {
      await btclgovernan.contract.actions.setparams.broadcast([
        fundingAccount,
        voteThreshold,
        votingDays,
        minimumBalanceToCreateProposals,
        proposalCost,
        approver
      ])
    } catch (error) {
      assert.strictEqual(
        error.message,
        'assertion failure with message: invalid token code'
      )
    }
  })

  it('should create a proposal', async () => {
    const creator = await eoslime.Account.createRandom(eosioAccount)
    const transferTx = await eosioToken.contract.actions.transfer.broadcast(
      [
        eosioAccount.name,
        creator.name,
        `1000.0000 ${SUPPORTED_TOKEN}`,
        'transfer'
      ],
      eosioAccount
    )
    const receiver = await eoslime.Account.createRandom(eosioAccount)
    const name = await eoslime.utils.randomName()
    const title = 'the best proposal'
    const detail = 'Lorem ipsum dolor sit amet, consectetur adipiscing elit.'
    const amount = `50000.0000 ${SUPPORTED_TOKEN}`
    const url = 'http://localhost:3000/'
    const createTx = await btclgovernan.contract.actions.create.broadcast(
      [creator.name, receiver.name, name, title, detail, amount, url],
      creator
    )
    const rows = await btclgovernan.contract.tables.proposals.equal(name).find()

    assert.strictEqual(rows[0].name, name)
    console.log(`    👍 transfer txid ${transferTx.transaction_id}`)
    console.log(`    👍 create txid ${createTx.transaction_id}`)
  })

  it('should return error on create proposal without balance', async () => {
    try {
      const creator = await eoslime.Account.createRandom(eosioAccount)
      const receiver = await eoslime.Account.createRandom(eosioAccount)
      const name = await eoslime.utils.randomName()
      const title = 'the best proposal'
      const detail = 'Lorem ipsum dolor sit amet, consectetur adipiscing elit.'
      const amount = `50000.0000 ${SUPPORTED_TOKEN}`
      const url = 'http://localhost:3000/'
      await btclgovernan.contract.actions.create.broadcast(
        [creator.name, receiver.name, name, title, detail, amount, url],
        creator
      )
    } catch (error) {
      assert.strictEqual(
        error.message,
        'assertion failure with message: not enough balance'
      )
    }
  })
})
