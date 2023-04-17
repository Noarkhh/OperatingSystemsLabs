CC=gcc
CFLAGS=-Wall

all: integralmain integralsub

integralmain:
	$(CC) $(CFLAGS) -o integral integralmain.c

integralsub:
	$(CC) $(CFLAGS) -o integralsub integralsub.c

clean:
	rm -f integralmain integralsub pipe

test:
	rm -f results.txt
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
