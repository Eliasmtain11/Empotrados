CC=gcc
CFLAGS= -Wall -Wshadow -Wvla
DEPS =

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
	
all: cyclictestURJC

cyclictestURJC: cyclictestURJC.c
	gcc -g -o cyclictestURJC cyclictestURJC.c

clean:
	rm -f *.o cyclictestURJC 