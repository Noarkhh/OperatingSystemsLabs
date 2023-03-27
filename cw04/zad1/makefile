CC=gcc
CFLAGS=-Wall

all: forkinh execinh

forkinh:
	$(CC) $(CFLAGS) -o forkinh siginh.c

execinh:
	$(CC) $(CFLAGS) -o execinh siginh.c -D MODE=2
	$(CC) $(CFLAGS) -o execinheritor execinheritor.c

clean:
	rm *a *o forkinh execinh
