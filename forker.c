// AUTHORS:
// MYKOLA KUSYY 
// GARRETT MCLAUGHLIN
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "target.h"
static char* getCommand(const char* terminalLine);
static const size_t MAX_LENGTH = 1024;

void runCommands(char* terminalLine){
		char *linuxCommand = getCommand(terminalLine);	
		int rc = fork();

		if(rc < 0){
			fprintf(stderr, "Fork failed, exiting");
			exit(-1);
		}
		else if(rc == 0){
			int success = execvp(linuxCommand, terminalLine);

			//shouldn't have reached here
			if(success == -1){
				fprintf(stderr, "Recipe failed at line\n%s", terminalLine);
				exit(-1);
			}
		}
}

static char* getCommand(const char* terminalLine){
	char* command = (char *) malloc(MAX_LENGTH);
	if( command == NULL){
		fprintf(stderr, "Failed to malloc in forkerc");
		exit(-1);
	}
	char ch = terminalLine[0];
	size_t i = 0;
	while(ch != ' ' && i < MAX_LENGTH){
		command[i] = ch;
		i++;
		ch = terminalLine[i];
	}	
	return command;

}
