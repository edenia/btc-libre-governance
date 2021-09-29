# EOSCR Backend Boilerplate

[![N|Solid](https://github.com/eoscostarica/eos-rate/raw/master/docs/eoscostarica-logo-black.png)](https://eoscostarica.io/)

[![JavaScript Style Guide](https://img.shields.io/badge/code_style-standard-brightgreen.svg)](https://standardjs.com) ![GitHub](https://img.shields.io/github/license/eoscostarica/backend-boilerplate) ![GitHub repo size](https://img.shields.io/github/repo-size/eoscostarica/backend-boilerplate) ![Twitter Follow](https://img.shields.io/twitter/follow/eoscostarica?style=social) ![GitHub forks](https://img.shields.io/github/forks/eoscostarica/backend-boilerplate?style=social)

A highly scalable skeleton with best practices, ideal for backend projects quick start

## Version

- 0.1.0

## Features!

This boilerplate features all the latest tools and practices in the industry

- **[hasura](https://hasura.io)**
  Hasura GraphQL Engine is an opensource service that connects to your databases & microservices and auto-generates a production-ready GraphQL backend

- **[hapi](https://hapi.dev/)**
  A back end service for custom busines logic integrated with hasura using [actions](https://hasura.io/docs/1.0/graphql/manual/actions/index.html#actions)

- **[docker-compose](https://docs.docker.com/compose/)**
  Compose is a tool for defining and running multi-container Docker applications

## File Structure

Within the download you'll find the following directories and files:

```bash
eoscrbackendboilerplate/
├── hasura
│ ├── migrations
│ └── config.yaml
├── hapi
│ ├── src
│ | ├── config
│ | ├── api
│ | ├── routes
│ | └── config.yaml
│ ├── .dockerignore
│ ├── .eslintrc
│ ├── .prettierrc
│ ├── Dockerfile
│ ├── yarn-lock.json
│ └── package.json
├── .env.example
├── .gitignore
├── docker-compose.yaml
├── .LICENSE
├── .prettierrc
├── Dockerfile
├── LICENSE
└── README.md
```

There are some important folders like

- `hapi/src/api` should have all reusable code for example a code to generate tax invoice
- `hapi/src/routes` this folder should only have the endpoint mapping and params validations and use functions from api folder to handle the business logic

## Installation

Basic knowledge about Docker, Docker Compose and NodeJS is required.

### Before to start

Somethings you need before getting started:

- [git](https://git-scm.com/)
- [node.js](https://nodejs.org/es/)
- [Hasura CLI](https://hasura.io/docs/1.0/graphql/manual/hasura-cli/install-hasura-cli.html#install-hasura-cli)

### First time

1.  Clone this repo using `git clone --depth=1 https://github.com/eoscostarica/backend-boilerplate.git <YOUR_PROJECT_NAME>`
2.  Move to the appropriate directory: `cd <YOUR_PROJECT_NAME>`.
3.  Copy the `.env.example` then update the environment variables according to your needs

### Quick start

At this point you can run `make run`, you can check the services runing on:

- hapi at http://localhost:9090
- hasura at http://localhost:9695

## License

MIT © [EOS Costa Rica](https://eoscostarica.io)

## Contributing

Please Read EOS Costa Rica's [Open Source Contributing Guidelines](https://developers.eoscostarica.io/docs/open-source-guidelines).

Please report bugs big and small by [opening an issue](https://github.com/eoscostarica/backend-boilerplate/issues)

## About EOS Costa Rica

<p align="center">
	<a href="https://eoscostarica.io">
		<img src="https://github.com/eoscostarica/eos-rate/raw/master/docs/eoscostarica-logo-black.png" width="300">
	</a>
</p>
<br/>

EOS Costa Rica is an independently-owned, self-funded, bare-metal Genesis block producer that provides stable and secure infrastructure for EOSIO blockchains. We support open source software for our community while offering enterprise solutions and custom smart contract development for our clients.

[eoscostarica.io](https://eoscostarica.io) Support OpenSource!
