include utils/meta.mk utils/help.mk

SHELL := /bin/bash
BLUE   := $(shell tput -Txterm setaf 6)
RESET  := $(shell tput -Txterm sgr0)

K8S_BUILD_DIR ?= ./build_k8s
K8S_FILES := $(shell find ./kubernetes -name '*.yaml' | sed 's:./kubernetes/::g')

run:
	make -B postgres
	make -B wallet
	make -B hapi
	make -B hasura
	make -B -j 3 hapi-logs hasura-cli

postgres:
	@docker-compose stop postgres
	@docker-compose up -d --build postgres
	@echo "$(BLUE)postgres |$(RESET) done"

wallet:
	@docker-compose stop wallet
	@docker-compose up -d --build wallet
	@echo "$(BLUE)wallet |$(RESET) done"

hapi:
	@docker-compose stop hapi
	@docker-compose up -d --build hapi
	@echo "$(BLUE)hapi |$(RESET) done"

hapi-logs:
	@docker-compose logs -f hapi

hasura:
	$(eval -include .env)
	@until \
		docker-compose exec -T postgres pg_isready; \
		do echo "$(BLUE)hasura |$(RESET) waiting for postgres service"; \
		sleep 5; done;
	@until \
		curl -s -o /dev/null -w 'hapi status %{http_code}\n' http://localhost:9090/healthz; \
		do echo "$(BLUE)hasura |$(RESET) waiting for hapi service"; \
		sleep 5; done;
	@docker-compose stop hasura
	@docker-compose up -d --build hasura
	@echo "$(BLUE)hasura |$(RESET) done"

hasura-cli:
	$(eval -include .env)
	@until \
		curl -s -o /dev/null -w 'hasura status %{http_code}\n' http://localhost:8080/healthz; \
		do echo "$(BLUE)hasura |$(RESET) waiting for hasura service"; \
		sleep 5; done;
	@cd hasura && hasura seeds apply --admin-secret $(HASURA_GRAPHQL_ADMIN_SECRET) && echo "success!" || echo "failure!";
	@cd hasura && hasura console --endpoint http://localhost:8080 --skip-update-check --no-browser --admin-secret $(HASURA_GRAPHQL_ADMIN_SECRET);

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

stop:
	@docker-compose stop

install: ##@local install dependencies
install:
	@cd ./hapi && yarn
	@npm install -g mocha
	@npm install -g eoslime
	@cd ./contracts && npm i

clean:
	@docker-compose stop
	@rm -rf tmp/db_data
	@rm -rf tmp/hapi
	@rm -rf hapi/node_modules
	@rm -rf tmp/wallet
	@docker system prune

build-kubernetes: ##@devops Generate proper k8s files based on the templates
build-kubernetes: ./kubernetes
	@echo "Build kubernetes files..."
	@rm -Rf $(K8S_BUILD_DIR) && mkdir -p $(K8S_BUILD_DIR)
	@for file in $(K8S_FILES); do \
		mkdir -p `dirname "$(K8S_BUILD_DIR)/$$file"`; \
		$(SHELL_EXPORT) envsubst <./kubernetes/$$file >$(K8S_BUILD_DIR)/$$file; \
	done

deploy-kubernetes: ##@devops Publish the build k8s files
deploy-kubernetes: $(K8S_BUILD_DIR)
	@kubectl create ns $(NAMESPACE) || echo "Namespace '$(NAMESPACE)' already exists.";
	@echo "Creating SSL certificates..."
	@kubectl create secret tls \
		tls-secret \
		--key ./ssl/eosio.cr.priv.key \
		--cert ./ssl/eosio.cr.crt \
		-n $(NAMESPACE)  || echo "SSL cert already configured.";
	@echo "Creating configmaps..."
	@kubectl create configmap -n $(NAMESPACE) \
	wallet-config \
	--from-file wallet/config/ || echo "Wallet configuration already created.";
	@echo "Applying kubernetes files..."
	@for file in $(shell find $(K8S_BUILD_DIR) -name '*.yaml' | sed 's:$(K8S_BUILD_DIR)/::g'); do \
		kubectl apply -f $(K8S_BUILD_DIR)/$$file -n $(NAMESPACE) || echo "${file} Cannot be updated."; \
	done

build-docker-images: ##@devops Build docker images
build-docker-images:
	@echo "Building docker containers..."
	@for dir in $(SUBDIRS); do \
		$(MAKE) build-docker -C $$dir; \
	done

push-docker-images: ##@devops Publish docker images
push-docker-images:
	@echo $(DOCKER_PASSWORD) | docker login \
		--username $(DOCKER_USERNAME) \
		--password-stdin
	for dir in $(SUBDIRS); do \
		$(MAKE) push-image -C $$dir; \
	done
