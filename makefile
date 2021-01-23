CC = gcc
CFLAGS = -o -wall
KEYLOGGER = keylogger.c
SERVER = server.c

.PHONY: all
all: keylogger server

keylogger: $(KEYLOGGER)
	$(CC) $^ -o $@

server: $(SERVER)
	$(CC) $^ -o $@

.PHONY: clean
clean:
	-rm -f keylogger server