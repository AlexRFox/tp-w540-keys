CFLAGS = -g -Wall
LIBS = `pkg-config --libs x11 xtst` -lcap

all: naga2014 gen-xmodmap

naga2014: naga2014.o
	$(CC) $(CFLAGS) -o naga2014 naga2014.o $(LIBS)

gen-xmodmap: gen-xmodmap.o
	$(CC) $(CFLAGS) -o gen-xmodmap gen-xmodmap.o $(LIBS)

caps: naga2014
	sudo setcap cap_dac_override+ep naga2014
	chmod 550 naga2014

install: naga2014 gen-xmodmap caps
	fix-xmodmap.sh
	cp `pwd`/btnmap ~/.btnmap

clean:
	rm -f naga2014 *~ *.o
