CFLAGS = -g -Wall

all: naga2014

naga2014: naga2014.o
	$(CC) $(CFLAGS) -o naga2014 naga2014.o

clean:
	rm -f naga2014 *~ *.o
