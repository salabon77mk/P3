CC=gcc
CFLAGS= -g -Wall -Wextra
DEPS = parser.h target.h cmd_parse.h mem_manage.h graphtrav.h forker.h 
OBJ =main.o parser.o cmd_parse.o  mem_manage.o target.o graphtrav.o forker.o
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

537make: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean
	
clean:
		rm *.o 537make

