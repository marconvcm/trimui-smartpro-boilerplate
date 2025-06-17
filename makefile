.PHONY: shell build run pak pak-zip clean cleanall

TOOLCHAIN_NAME=tg5040-toolchain
WORKSPACE_DIR := $(shell pwd)/workspace
BUILD_DIR := $(shell pwd)/.build

CONFIG_FILE := $(shell pwd)/config.ini
PROJECT_NAME := $(shell grep '^name' $(CONFIG_FILE) | cut -d '=' -f 2 | tr -d ' ')
PROJECT_VERSION := $(shell grep '^version' $(CONFIG_FILE) | cut -d '=' -f 2 | tr -d ' ')
TARGET := tg5040_app

CONTAINER_NAME=$(shell docker ps -f "ancestor=$(TOOLCHAIN_NAME)" --format "{{.Names}}")
BOLD=$(shell tput bold)
NORM=$(shell tput sgr0)

.build: Dockerfile
	$(info $(BOLD)Building $(TOOLCHAIN_NAME)...$(NORM))
	mkdir -p ./workspace
	docker build -t $(TOOLCHAIN_NAME) .
	mkdir .build

ifeq ($(CONTAINER_NAME),)
shell: .build
	$(info $(BOLD)Starting $(TOOLCHAIN_NAME)...$(NORM))
	docker run -it --rm -v "$(WORKSPACE_DIR)":/root/workspace $(TOOLCHAIN_NAME) /bin/bash
else
shell:
	$(info $(BOLD)Connecting to running $(TOOLCHAIN_NAME)...$(NORM))
	docker exec -it $(CONTAINER_NAME) /bin/bash  
endif

build: .build
	$(info $(BOLD)Building application inside $(TOOLCHAIN_NAME)...$(NORM))
	@mkdir -p $(BUILD_DIR) || true
	@cp -f $(CONFIG_FILE) $(BUILD_DIR)/ || true
ifeq ($(CONTAINER_NAME),)
	docker run --rm -v "$(WORKSPACE_DIR)":/root/workspace -v "$(BUILD_DIR)":/root/build -e CONFIG_INI=/root/build/config.ini $(TOOLCHAIN_NAME) make -C /root/workspace
	@cp -f $(WORKSPACE_DIR)/.build/$(TARGET) $(BUILD_DIR)/ 2>/dev/null || true
else
	docker exec -it $(CONTAINER_NAME) make -C /root/workspace
	@cp -f $(WORKSPACE_DIR)/.build/$(TARGET) $(BUILD_DIR)/ 2>/dev/null || true
endif

run: .build
	$(info $(BOLD)Running application inside $(TOOLCHAIN_NAME)...$(NORM))
ifeq ($(CONTAINER_NAME),)
	docker run --rm -v "$(WORKSPACE_DIR)":/root/workspace $(TOOLCHAIN_NAME) make -C /root/workspace run
else
	docker exec -it $(CONTAINER_NAME) make -C /root/workspace run
endif

pak: build
	$(info $(BOLD)Creating PAK structure inside $(TOOLCHAIN_NAME)...$(NORM))
	@mkdir -p $(BUILD_DIR)
ifeq ($(CONTAINER_NAME),)
	docker run --rm -v "$(WORKSPACE_DIR)":/root/workspace -v "$(BUILD_DIR)":/root/build -e CONFIG_INI=/root/build/config.ini $(TOOLCHAIN_NAME) make -C /root/workspace pak
	@cp -rf $(WORKSPACE_DIR)/.output/$(PROJECT_NAME).pak $(BUILD_DIR)/ 2>/dev/null || true
else
	docker exec -it $(CONTAINER_NAME) make -C /root/workspace pak
	@cp -rf $(WORKSPACE_DIR)/.output/$(PROJECT_NAME).pak $(BUILD_DIR)/ 2>/dev/null || true
endif

pak-zip: pak
	$(info $(BOLD)Creating PAK zip package inside $(TOOLCHAIN_NAME)...$(NORM))
	@mkdir -p $(BUILD_DIR)
ifeq ($(CONTAINER_NAME),)
	docker run --rm -v "$(WORKSPACE_DIR)":/root/workspace -v "$(BUILD_DIR)":/root/build -e CONFIG_INI=/root/build/config.ini $(TOOLCHAIN_NAME) make -C /root/workspace pak-zip
	@cp -f $(WORKSPACE_DIR)/.output/$(PROJECT_NAME)_v$(PROJECT_VERSION).tar.gz $(BUILD_DIR)/ 2>/dev/null || true
else
	docker exec -it $(CONTAINER_NAME) make -C /root/workspace pak-zip
	@cp -f $(WORKSPACE_DIR)/.output/$(PROJECT_NAME)_v$(PROJECT_VERSION).tar.gz $(BUILD_DIR)/ 2>/dev/null || true
endif

clean:
	$(info $(BOLD)Cleaning application build...$(NORM))
ifeq ($(CONTAINER_NAME),)
	docker run --rm -v "$(WORKSPACE_DIR)":/root/workspace $(TOOLCHAIN_NAME) make -C /root/workspace clean
else
	docker exec -it $(CONTAINER_NAME) make -C /root/workspace clean
endif
	@echo "Cleaning workspace build artifacts..."
	@rm -rf $(WORKSPACE_DIR)/.build
	@rm -rf $(WORKSPACE_DIR)/.output

cleanall: clean
	$(info $(BOLD)Removing build directory and docker image...$(NORM))
	@rm -rf $(BUILD_DIR)
	docker rmi $(TOOLCHAIN_NAME)
	rm -f .build
