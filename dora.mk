CC = clang
CFLAGS = `pkg-config --cflags --libs libnotify`

default: all

all: Dora

Dora: dora.c
	$(CC) $(CFLAGS) -o dora dora.c
