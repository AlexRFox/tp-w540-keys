CFLAGS = -g -Wall
LIBS = `pkg-config --libs x11 xtst` -lcap

all: naga2014

naga2014: naga2014.o
	$(CC) $(CFLAGS) -o naga2014 naga2014.o $(LIBS)

caps: naga2014
	sudo setcap cap_dac_override+ep naga2014

clean:
	rm -f naga2014 *~ *.o
