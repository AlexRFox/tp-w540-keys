CFLAGS = -g -Wall

all: rasp

rasp: rasp.o
	$(CC) $(CFLAGS) -o rasp rasp.o

valgrind: rasp
	valgrind --leak-check=full --show-reachable=yes ./rasp

clean:
	rm -f rasp *~ *.o
