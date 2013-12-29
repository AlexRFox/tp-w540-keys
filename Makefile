CFLAGS = -g -Wall
LIBS = `pkg-config --libs x11 xtst`

all: naga2014

naga2014: naga2014.o
	$(CC) $(CFLAGS) -o naga2014 naga2014.o $(LIBS)

clean:
	rm -f naga2014 *~ *.o
