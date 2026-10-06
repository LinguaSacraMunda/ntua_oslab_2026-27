CC = gcc
CFLAGS = -O2 -pthread
LIBS =

all: m

m: m.o
	$(CC) $(CFLAGS) -o m m.o $(LIBS)

m.o: m.c
	$(CC) $(CFLAGS) -c -o m.o m.c

clean:
	rm -f *.s *.o m
