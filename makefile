include utils/meta.mk utils/help.mk

SHELL := /bin/bash
BLUE   := $(shell tput -Txterm setaf 6)
RESET  := $(shell tput -Txterm sgr0)

run:
	make -B test-contracts

stop:
	@docker-compose stop

testnet:
	@docker-compose stop testnet
	@docker container rm testnet -f > /dev/null 2>&1 || echo ""
	@docker-compose up -d --build testnet
	@until \
		cleos get info > /dev/null 2>&1; \
		do echo "$(BLUE)testnet |$(RESET) ..."; \
		sleep 5; done;
	@echo "$(BLUE)testnet |$(RESET) done"

build-contracts:
	@mkdir -p contracts/btclgovernan/build
	@rm -rf contracts/btclgovernan/build/*
	@cd contracts/btclgovernan/build && cmake .. && make

deploy-contracts:
	$(eval -include .env)
	@mkdir -p tmp/contracts
	@cleos wallet unlock --name $(CONTRACTS_BTCLGOVERNAN_ACCOUNT) --password $(CONTRACTS_BTCLGOVERNAN_PASSWORD) || echo ""
	@cleos -u $(CONTRACTS_CHAIN_ENDPOINT) set contract $(CONTRACTS_BTCLGOVERNAN_ACCOUNT) ./contracts/btclgovernan/build/btclgovernan || echo ""
	@cat "contracts/btclgovernan/permission.json" | sed -e 's/<CONTRACT_ACCOUNT>/${CONTRACTS_BTCLGOVERNAN_ACCOUNT}/g' > "tmp/contracts/btclgovernan_permission_temp.json"
	@cat "tmp/contracts/btclgovernan_permission_temp.json" | sed -e 's/<CONTRACT_PUBLIC_KEY>/${CONTRACTS_BTCLGOVERNAN_PUBLIC_KEY}/g' > "tmp/contracts/btclgovernan_permission.json"
	@cleos -u $(CONTRACTS_CHAIN_ENDPOINT) set account permission $(CONTRACTS_BTCLGOVERNAN_ACCOUNT) active tmp/contracts/btclgovernan_permission.json owner -p $(CONTRACTS_BTCLGOVERNAN_ACCOUNT)
	@cleos wallet lock --name $(CONTRACTS_BTCLGOVERNAN_ACCOUNT)
	@rm -rf tmp/contracts/

test-contracts:
	$(eval -include .env)
	make -B testnet
	@until \
		cleos get account eosio.token > /dev/null 2>&1; \
		do echo "$(BLUE)run-contracts-tests |$(RESET) waiting for testnet service"; \
		sleep 5; done;
	@sleep 1.5
	@cd contracts/tests && npm run test

install: ##@local install dependencies
install:
	@npm install -g mocha
	@npm install -g eoslime
	@cd ./contracts && npm i

clean:
	@rm -rf tmp
	@docker system prune --all
