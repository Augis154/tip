CFLAGS = -ggdb

ASM_SRC = $(wildcard assembler/*.c)
ASM_OBJ = $(patsubst assembler/%.c, build/assembler/%.o, $(ASM_SRC))

all: as

$(ASM_OBJ): build/assembler/%.o: assembler/%.c | build
	gcc $(CFLAGS) -c $< -o $@

as: $(ASM_OBJ)
	gcc $(CFLAGS) $^ -o as

build:
	mkdir -p build
	mkdir -p build/assembler


.PHONY: clean

clean:
	rm -r build
