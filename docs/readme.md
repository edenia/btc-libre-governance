## BTC Libre Governance 

The system works as a decentralized voting mechanism set up in the rules governing the blockchain, where budgets for specific projects are proposed and funded once they receive sufficient votes from token holders.

Token holders can create a proposal to be paid in Bitcoin. The cost of creating a proposal is **50,000 SATS** users must also hold a minimum of **1,000 LIBRE** balance in order to submit a proposal.

Active proposals will be voted on by LIBRE token holders who can vote for or against proposals during a **10 day** voting window. Token holders can vote with a voting wight that corresponds to their balance.

All proposals are subject to a 10 day voting period, and any address with voting power can vote for or against the proposal. If a majority, and at least 10% of circulating supply votes are cast for the proposal, it is queued for approval, and can be executed or flagged and canceled.

![Governance Flow](btc-libre-governance.png)

# Contract Data Model

![Data Model](data-model.png)

# Contract Actions

|    User Role    |   Action    |        Description         |               Pre Conditions                 |          Post Conditions            |
| :-------------: | :---------: | :-----------------------:  | :-----------------------------------------:  | :-------------------------------:   |
| Proposer  | `create`  | Create a new proposal    | Account must pay fee and hold `1000 LIBRE`      | Proposal is created and active for voting   |
| Voter  | `vote_for`  | Vote for an active proposal     | proposal must be active     | Vote for count increases  |
| Voter  | `vote_against`  | Vote against an active proposal     | proposal must be active     | Vote against count increases|
| Approver  | `approve`  | Approve a Proposal    | Proposal must have succeeded      | Proposal is executed  |
| Approver  | `reject`  | Reject a Proposal    | Proposal must have succeeded      | Proposal is executed  |

## Proposals

Proposals are executable code as a funds transfer to the receiver account. 

### Proposal Fields 

- Proposer Account
- Proposal Name
- Proposal Detail
- Proposal Amount in Sats
- Account to be Paid


### Proposal Status

```
  ACTIVE = 1
  SUCCEEDED = 2
  DEFEATED = 3
  CANCELED = 3
```

### Proposal Cost

In order to prevent spam and ensure only serious proposals make it to this stage. a registration fee of **50,000** sats must be paid for any proposal to be created.

## Voting

BTC Libre is managed by a decentralized community of LIBRE token holders who propose and vote on projects for the network.

### Voting period
All proposals are subject to a 10 day voting period, and any address with voting power can vote for or against the proposal. If a majority, and at least 10% of circulating supply votes are cast for the proposal, it is queued in the Timelock, and can be implemented after 2 days. In the timelock period proposals can be flagged and canceled

### Voting Threshold

10% of the circulating supply must vote in order for a proposal to be eligible for acceptance. If more than 50% of token 

## Proposal Approval

### Proposal Execution

### Proposal Cancellation

