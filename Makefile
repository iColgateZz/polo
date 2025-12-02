CFLAGS=-Wall -Wextra -O0 -Iinclude -g
CC=gcc

SRC := $(shell find . -name "*.c")
OBJ := $(patsubst %,build/%,$(SRC:.c=.o))
DEP := $(OBJ:.o=.d)
EXE := polo

.PHONY: all clean

all: $(EXE)

build/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

clean:
	rm -rf build $(EXE)

-include $(DEP)
