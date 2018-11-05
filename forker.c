// AUTHORS:
// MYKOLA KUSYY 
// GARRETT MCLAUGHLIN
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "target.h"


static void printCommands(char** commands, size_t numArgs);

void runCommands(char** commands, char* target, size_t numArgs){
	pid_t pid = fork();

	if(pid < 0){
		fprintf(stderr, "Fork failed, exiting");
		exit(-1);
	}
	else if(pid == 0){
		printCommands(commands, numArgs);
		int success = execvp(commands[0], commands);

		//shouldn't have reached here
		if(success == -1){
			fprintf(stderr, "Recipe failed for recipe \n%s", target);
			exit(-1);
		}
	}
	else{
		wait(NULL);
	}
}


static void printCommands(char** commands, size_t numArgs){
	for(size_t i = 0; i < numArgs; i++){
		printf("%s ", commands[i]);
	}
	printf("\n");
}

