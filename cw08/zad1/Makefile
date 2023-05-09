CC = gcc
CFLAGS = -Wall

all: clean life
	
life: grid.c life.c
	$(CC) $(CFLAGS) grid.c life.c -o life -lncursesw
	
clean:
	rm -f life
