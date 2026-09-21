CC := gcc
CFLAGS := -Iinclude
TARGET := twcc
SRCS := $(wildcard src/*.c)
OBJS := $(patsubst src/%.c,build/%.o,$(SRCS))
JOBS := $(shell nproc)

.PHONY: all build clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	$(MAKE) -j$(JOBS)

clean:
	rm -rf build $(TARGET)