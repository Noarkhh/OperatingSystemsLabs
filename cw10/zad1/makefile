CC=gcc
CFLAGS=-Wall

all: clean server client

server:
	$(CC) $(CFLAGS) -o server server.c

client:
	$(CC) $(CFLAGS) -o client client.c

clean:
	rm -f server client
