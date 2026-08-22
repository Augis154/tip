CFLAGS = -ggdb

CURRENT_DIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
BUILD_DIR = build

ASM_SRC = $(wildcard assembler/*.c assembler/lib/*.c)
ASM_OBJ = $(patsubst %.c, $(BUILD_DIR)/%.o, $(ASM_SRC))

all: as

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

as: $(ASM_OBJ)
	gcc $(CFLAGS) $^ -o as

build:
	mkdir -p $(BUILD_DIR)
	
	mkdir -p $(BUILD_DIR)/assembler
	mkdir -p $(BUILD_DIR)/assembler/lib

.PHONY: clean

clean:
	rm -r $(BUILD_DIR)

install-syntax-highlighter:
	ln -sfn "$(CURRENT_DIR)/syntax-highlight/tip-asm-vscode" "$$HOME/.vscode/extensions/tip-asm-vscode"
