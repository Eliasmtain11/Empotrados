CC=gcc
CFLAGS=-Wall -Wshadow -Wvla -g 
LDLIBS=-pthread -lrt
DEPS=

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: cyclictestURJC

cyclictestURJC: cyclictestURJC.o
	$(CC) -o $@ $^ $(LDLIBS)

clean:
	rm -f *.o cyclictestURJC

