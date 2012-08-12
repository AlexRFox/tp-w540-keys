CFLAGS = -g -Wall

all: rasp keyread

rasp: rasp.o
	$(CC) $(CFLAGS) -o rasp rasp.o

keyread: keyread.o
	$(CC) $(CFLAGS) -o keyread keyread.o

valgrind: rasp
	valgrind --leak-check=full --show-reachable=yes ./rasp

clean:
	rm -f rasp *~ *.o
