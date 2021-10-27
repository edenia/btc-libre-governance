<p align="center">
   <img src="../../docs/proton-logo.svg" width="200">
</p>

# BTC Libre Governance Smart Contract

View [project readme](../../README.md) document.

The **btclgovernan** smart contract works as a decentralized voting mechanism set up in the rules governing the blockchain, where budgets for specific projects are proposed and funded once they receive sufficient votes from token holders.

# Data Model

Data persisted in the smart contract multi index tables

![Data Model](../../docs/data-model.png)

# Actions

Actions available to be executed

|   User Role    |     Action     |                   Description                   |                                 Pre Conditions                                 |           Post Conditions            |
| :------------: | :------------: | :---------------------------------------------: | :----------------------------------------------------------------------------: | :----------------------------------: |
|    Proposer    |    `create`    |              Create a new proposal              |                         Account must hold `1000 LIBRE`                         |     Proposal is created as draft     |
|    Proposer    |   `transfer`    |             pay for a new proposal              |                    Account must pay cost of a new proposal                     | Proposal is set as active for voting |
|     Voter      |   `votefor`   |           Vote for an active proposal           | proposal must be active , voter must be registered and have a non-zero balance |       Vote for count increases       |
|     Voter      | `voteagainst` |         Vote against an active proposal         | proposal must be active , voter must be registered and have a non-zero balance |     Vote against count increases     |
|    Approver    |   `approve`    |               Approve a Proposal                |                          Proposal must have succeeded                          |         Proposal is executed         |
|    Approver    |    `reject`    |                Reject a Proposal                |                          Proposal must have succeeded                          |         Proposal is executed         |
| Smart Contract | `countvotes`  | Count votes to determine if a proposal succeeds |                   Proposal must have completed voting window                   |  Proposal is approved or cancelled   |

# Unit tests

Tests were designed to validate the affiliate smart contracts actions, to run it follow those steps:

- If Node.js is not installed on your system, install the Node.js package by typing:
  `brew install node`

- Install dependencies:
  `make install`

- Run tests
  `make test-contracts`
