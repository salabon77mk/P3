537make: main.o parser.o cmd_parse.o mem_manage.o target.o graphtrav.o forker.o 
	gcc -o 537make main.o parser.o cmd_parse.o mem_manage.o target.o graphtrav.o forker.o -g -Wall -Wextra

main.o: main.c target.h parser.h graphtrav.h cmd_parse.h mem_manage.h
	gcc -c main.c -g -Wall -Wextra

parser.o: parser.c parser.h target.h mem_manage.h sizes_struct.h
	gcc -c parser.c -g -Wall -Wextra

cmd_parse.o: cmd_parse.c cmd_parse.h
	gcc -c cmd_parse.c -g -Wall -Wextra

mem_manage.o: mem_manage.c
	gcc -c mem_manage.c -g -Wall -Wextra

target.o: target.c target.h parser.h mem_manage.h
	gcc -c target.c -g -Wall -Wextra

graphtrav.o: graphtrav.c target.h parser.h forker.h mem_manage.h
	gcc -c graphtrav.c -g -Wall -Wextra

forker.o: forker.c target.h
	gcc -c forker.c -g -Wall -Wextra

cleanALL:
	rm *.o 537make
clean:
	rm main.o parser.o cmd_parse.o mem_manage.o target.o graphtrav.o forker.o -g -Wall -Wextra

