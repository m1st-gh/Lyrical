CC = clang
CFLAGS = -Wall -Wextra -g -lcoglink -ldiscord -lcurl -pthread

.PHONY: all clean

all: lyrical

OBJ_DIR = obj
EXEC_DIR = out

OBJS = $(OBJ_DIR)/main.o $(OBJ_DIR)/music.o

lyrical: $(OBJS)
	$(CC) $(OBJS) -o $(EXEC_DIR)/Lyrical $(CFLAGS)

$(OBJ_DIR)/main.o: src/main.c src/lyrical.h
	$(CC) -c src/main.c -o $(OBJ_DIR)/main.o 

$(OBJ_DIR)/music.o: src/music.c src/lyrical.h
	$(CC) -c src/music.c -o $(OBJ_DIR)/music.o 

.PHONY: all clean

all: lyrical

clean:
	rm -rf $(OBJ_DIR) $(EXEC_DIR)

clean:
	rm -f lyrical
