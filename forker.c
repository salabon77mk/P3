// AUTHORS:
// MYKOLA KUSYY 
// GARRETT MCLAUGHLIN
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "target.h"


void runCommands(char** commands, char* target){
		pid_t pid = fork();

		if(pid < 0){
			fprintf(stderr, "Fork failed, exiting");
			exit(-1);
		}
		else if(pid == 0){
			int i = 0;
			char* command = commands[i];
			while(command != NULL){
				printf("%s ", command);
				i++;
				command = commands[i];
			}
			printf("\n");

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
