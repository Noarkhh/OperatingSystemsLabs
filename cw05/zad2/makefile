CC=gcc
CFLAGS=-Wall

all: integral

integral:
	$(CC) $(CFLAGS) -o integral integral.c

clean:
	rm integral

test:
	rm results.txt
	make clean
	make all
	echo '\n./integral 50000000 1' >> results.txt
	./integral 50000000 1 >> results.txt
	echo '\n./integral 50000000 2' >> results.txt
	./integral 50000000 2 >> results.txt
	echo '\n./integral 50000000 10' >> results.txt
	./integral 50000000 10 >> results.txt
	echo '\n./integral 50000000 100' >> results.txt
	./integral 50000000 100 >> results.txt
	echo '\n./integral 700000000 1' >> results.txt
	./integral 700000000 1 >> results.txt
	echo '\n./integral 700000000 2' >> results.txt
	./integral 700000000 2 >> results.txt
	echo '\n./integral 700000000 10' >> results.txt
	./integral 700000000 10 >> results.txt
	echo '\n./integral 700000000 100' >> results.txt
	./integral 700000000 100 >> results.txt
