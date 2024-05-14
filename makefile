CC = clang
CFLAGS = -Wall -Wextra -g -O3
LDFLAGS = -lcoglink -ldiscord -lcurl -pthread

OBJ_DIR = obj
EXEC_DIR = out

SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SRCS))

.PHONY: all clean objs

all: dirs lyrical

dirs:
	@if [ ! -d $(OBJ_DIR) ]; then mkdir -p $(OBJ_DIR); fi
	@if [ ! -d $(EXEC_DIR) ]; then mkdir -p $(EXEC_DIR); fi

lyrical: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC_DIR)/Lyrical $(LDFLAGS)

objs: dirs $(OBJS)

$(OBJ_DIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)/* $(EXEC_DIR)/*