/**
 * @file Unit test for btclgovernan program {@link https://github.com/eoscostarica/btc-libre-governance}.
 * @author EOSCostaRica <https://info@eoscostarica.io>
 * @version 1.1.0
 */
const assert = require('assert')

const utils = require('./utils')

const BTCLGOVERNAN_WASM_PATH = './btclgovernan/btclgovernan.wasm'
const BTCLGOVERNAN_ABI_PATH = './btclgovernan/btclgovernan.abi'

let eoslime
let eosioAccount
let btclgovernanAccount
let btclgovernanContract

describe('btclgovernan contract', function () {
  this.timeout(15000)

  const btclgovernanDeploy = async () => {
    try {
      btclgovernanAccount = await eoslime.Account.create(
        'btclgovernan',
        process.env.TESTNET_EOSIO_PRIVATE_KEY,
        eosioAccount
      )
      btclgovernanContract = await eoslime.Contract.deployOnAccount(
        BTCLGOVERNAN_WASM_PATH,
        BTCLGOVERNAN_ABI_PATH,
        btclgovernanAccount,
        { inline: true }
      )
    } catch (error) {
      btclgovernanAccount = eoslime.Account.load(
        'btclgovernan',
        process.env.TESTNET_EOSIO_PRIVATE_KEY
      )
      btclgovernanContract = await eoslime.Contract.fromAccount(
        btclgovernanAccount
      )
    }
  }

  before(async () => {
    eoslime = utils.eoslimeInit()
    eosioAccount = eoslime.Account.load(
      process.env.TESTNET_EOSIO_ACCOUNT,
      process.env.TESTNET_EOSIO_PRIVATE_KEY
    )
    await btclgovernanDeploy()
  })

  it('Should be same account name', () => {
    assert.strictEqual(btclgovernanAccount.name, btclgovernanContract.name)
  })
})
