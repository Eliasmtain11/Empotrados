CC=gcc
CFLAGS=-I -Wall -Wshadow -Wvla
DEPS =

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
	
all: cyclictestURJC

server: cyclictestURJC.c
	gcc -g -o cyclictestURJC cyclictestURJC.c

clean:
	rm -f *.o cyclictestURJC