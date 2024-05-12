CC = clang
CFLAGS = -Wall -Wextra -g -lcoglink -ldiscord -lcurl -pthread

.PHONY: all clean

all: lyrical

lyrical: src/main.c src/music.c src/lyrical.h
	$(CC) src/main.c src/music.c -o lyrical $(CFLAGS)

clean:
	rm -f lyrical
