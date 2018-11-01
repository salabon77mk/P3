CC=gcc
CFLAGS= -g
DEPS = parser.h target.h graphtrav.h forker.h 
OBJ =main.o parser.o target.o graphtrav.o forker.o
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

537make: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean
	
clean:
		rm *.o 537make

