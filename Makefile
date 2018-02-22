CC=gcc
CFLAGS=-Wall -O0 -g -I../C-Thread-Pool/
LDFLAGS=-rdynamic -lpthread
EXEC=main
# SRC=$(wildcard *.c)
SRC=main.c rcsv.c buffer_pool.c ../C-Thread-Pool/thpool.c
OBJ=$(SRC:.c=.o)

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)
clean:
	rm -f *.o core

mrproper: clean
	rm -f $(EXEC)
