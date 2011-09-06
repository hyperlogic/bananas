# Makefile for Mac OS 10.6.8 (snow-leopard)

GCC = gcc

CFLAGS = -Wall -g -DKERNEL # -DGC_DEBUG
LFLAGS = -lc -lreadline

OBJ = bananas.o parse.o prim.o obj.o symbol.o

HEADERS = Makefile parse.h prim.h obj.h symbol.h

bananas: $(OBJ)
	$(GCC) $(OBJ) -o bananas $(LFLAGS)

bananas.o: bananas.c $(HEADERS)
	$(GCC) $(CFLAGS) -c $<

parse.o: parse.c $(HEADERS)
	$(GCC) $(CFLAGS) -c $<

prim.o: prim.c $(HEADERS)
	$(GCC) $(CFLAGS) -c $<

obj.o: obj.c $(HEADERS)
	$(GCC) $(CFLAGS) -c $<

symbol.o: symbol.c $(HEADERS)
	$(GCC) $(CFLAGS) -c $<

clean:
	rm $(OBJ) bananas
