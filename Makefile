# Makefile for Mac OS 10.6.8 (snow-leopard)

GCC = gcc

CFLAGS = -Wall -O
LFLAGS = -lc -lreadline

OBJ = bananas.o parse.o

bananas: $(OBJ)
	$(GCC) $(OBJ) -o bananas $(LFLAGS)

bananas.o: bananas.c parse.h
	$(GCC) $(CFLAGS) -c $<

parse.o: parse.c parse.h
	$(GCC) $(CFLAGS) -c $<

clean:
	rm $(OBJ) bananas
