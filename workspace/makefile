# Makefile for TG5040 application
# This will be run inside the Docker container

# Compiler and flags
CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -Wextra -O2 -I$(SRC_DIR)
LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lm

# Read project name and version from config.ini
CONFIG_INI ?= ../config.ini
ifneq ("$(wildcard $(CONFIG_INI))","")
  PROJECT_NAME := $(shell grep '^name' $(CONFIG_INI) | cut -d '=' -f 2 | tr -d ' ')
  PROJECT_VERSION := $(shell grep '^version' $(CONFIG_INI) | cut -d '=' -f 2 | tr -d ' ')
  PROJECT_DESCRIPTION := $(shell grep '^description' $(CONFIG_INI) | cut -d '=' -f 2)
  PROJECT_AUTHOR := $(shell grep '^author' $(CONFIG_INI) | cut -d '=' -f 2)
endif

# Directories
SRC_DIR = src
RES_DIR = res
BUILD_DIR = .build
OUTPUT_DIR = .output
PAK_DIR = $(OUTPUT_DIR)/$(PROJECT_NAME).pak

# Output binary name
TARGET = $(BUILD_DIR)/tg5040_app
PAK_NAME = $(PROJECT_NAME)
PAK_TAG = $(shell echo $(PROJECT_NAME) | tr '[:upper:]' '[:lower:]' | tr -d ' ')
PAK_VERSION = $(PROJECT_VERSION)

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

# Default target
all: directories $(TARGET)

# Create necessary directories
directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(OUTPUT_DIR)

# Link the application
$(TARGET): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(OUTPUT_DIR)

# Run target (only for testing within Docker)
run: $(TARGET)
	$(TARGET)

# Create PAK structure for MinUI
pak: $(TARGET)
	@echo "Creating PAK structure for $(PAK_NAME) v$(PAK_VERSION)..."
	@mkdir -p $(PAK_DIR)
	@mkdir -p $(PAK_DIR)/res
	@cp $(TARGET) $(PAK_DIR)/
	
	@echo "Copying resources..."
	@cp -r $(RES_DIR)/* $(PAK_DIR)/res/ 2>/dev/null || true
	
	@echo "Creating launch.sh script..."
	@echo '#!/bin/sh' > $(PAK_DIR)/launch.sh
	@echo '' >> $(PAK_DIR)/launch.sh
	@echo 'APP_EXE=tg5040_app' >> $(PAK_DIR)/launch.sh
	@echo 'CORES_PATH=$$(dirname "$$0")' >> $(PAK_DIR)/launch.sh
	@echo '' >> $(PAK_DIR)/launch.sh
	@echo '###############################' >> $(PAK_DIR)/launch.sh
	@echo '' >> $(PAK_DIR)/launch.sh
	@echo 'APP_TAG=$$(basename "$$(dirname "$$0")" .pak)' >> $(PAK_DIR)/launch.sh
	@echo 'HOME="$$USERDATA_PATH"' >> $(PAK_DIR)/launch.sh
	@echo 'mkdir -p "$$SAVES_PATH/$$APP_TAG"' >> $(PAK_DIR)/launch.sh
	@echo 'mkdir -p "$$LOGS_PATH"' >> $(PAK_DIR)/launch.sh
	@echo 'cd "$$CORES_PATH"' >> $(PAK_DIR)/launch.sh
	@echo './$$APP_EXE &> "$$LOGS_PATH/$$APP_TAG.txt"' >> $(PAK_DIR)/launch.sh
	@chmod +x $(PAK_DIR)/launch.sh
	
	@echo "Creating metadata.txt..."
	@echo 'name: $(PAK_NAME)' > $(PAK_DIR)/metadata.txt
	@echo 'version: $(PAK_VERSION)' >> $(PAK_DIR)/metadata.txt
	@echo 'author: $(PROJECT_AUTHOR)' >> $(PAK_DIR)/metadata.txt
	@echo 'description: $(PROJECT_DESCRIPTION)' >> $(PAK_DIR)/metadata.txt
	
	@echo "PAK structure created at $(PAK_DIR)"

# Package PAK into zip file
pak-zip: pak
	@echo "Creating ZIP package..."
	@mkdir -p $(OUTPUT_DIR)
	@cd $(OUTPUT_DIR) && tar -czf $(PAK_NAME)_v$(PAK_VERSION).tar.gz $(PAK_NAME).pak
	@echo "Package created at $(OUTPUT_DIR)/$(PAK_NAME)_v$(PAK_VERSION).tar.gz"

.PHONY: all clean run pak pak-zip directories
