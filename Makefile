CFLAGS = -g -Wall

all: keycast keyread

keycast: keycast.o
	$(CC) $(CFLAGS) -o keycast keycast.o

keyread: keyread.o
	$(CC) $(CFLAGS) -o keyread keyread.o

valgrind: keycast
	valgrind --leak-check=full --show-reachable=yes ./keycast

clean:
	rm -f keycast *~ *.o
