CC = clang
CFLAGS = `pkg-config --cflags --libs libnotify`
default: all

all: dora doractl

dora: dora.c
	$(CC) $(CFLAGS) -o dora dora.c

doract: doractl.c
	$(CC) $(CFLAGS) -o doractl doractl.c
