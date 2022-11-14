CC = gcc
CFLAGS = -o -wall
VICTIM = victim.c
SERVER = server.c

.PHONY: all
all: victim server

victim: $(VICTIM)
	$(CC) $^ -o $@

server: $(SERVER)
	$(CC) $^ -o $@

.PHONY: clean
clean:
	-rm -f victim server