CC = clang
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lcoglink -ldiscord -lcurl -pthread

OBJ_DIR = obj

SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SRCS))

.PHONY: all clean objs

all: dirs lyrical

dirs:
	@if [ ! -d $(OBJ_DIR) ]; then mkdir -p $(OBJ_DIR); fi

lyrical: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o Lyrical $(LDFLAGS)

objs: dirs $(OBJS)

$(OBJ_DIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)/*
	rm -f Lyrical