CC := gcc
CFLAGS := -Iinclude -MMD -MP
TARGET := twcc
SRCS := $(wildcard src/*.c)
OBJS := $(patsubst src/%.c,build/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)
JOBS := $(shell nproc)

.PHONY: all build clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

build:
	$(MAKE) -j$(JOBS)

clean:
	rm -rf build $(TARGET)