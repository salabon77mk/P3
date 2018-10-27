CC=gcc
CFLAGS= -g
DEPS = parser.h target.h
OBJ =main.o parser.o target.o 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

537make: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean
	
clean:
		rm *.o 537make

