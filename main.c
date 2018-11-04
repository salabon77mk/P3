// AUTHORS:
// MYKOLA KUSYY 
// GARRETT MCLAUGHLIN
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include "target.h"
#include "parser.h"
#include "graphtrav.h"

static char* getMake();
static struct Rules* getRules(const char* makefile);


int main(void){
	const char* makefile = getMake();
	struct Rules* rules = getRules(makefile);
	rules = createGraph(rules);	
//	build(rules->rules[0]);

	printf("Stop here debugger!");
}

// Find a makefile
// If it exists, we will be able to safely open it and pass it to parser
static char* getMake(){
	//TODO Should be easy to implement extra credit with an arg here
	DIR *dirp = opendir(".");
	struct dirent *entry;

	
	char* lilMake = "testmake"; //CHANGE TO makefile WHEN FINISHED
	char* bigMake = "Makefile";
	//extra credit const char* targetFile

	while((entry = readdir(dirp)) != NULL){
		if(!strcmp(entry->d_name, lilMake)){
			closedir(dirp);
			return lilMake;
		}

		if(!strcmp(entry->d_name, bigMake)){
			closedir(dirp);
			return bigMake;
		}
	}

	closedir(dirp);
	return NULL; //failed to find
}

static struct Rules* getRules(const char* makefile){
	FILE *fptr;

	fptr = fopen(makefile, "r");

	if(fptr == NULL){
		fprintf(stderr, "Failed to open makefile, exiting");
		exit(0);
	}
	struct Rules* rules = parseRules(fptr);
	fclose(fptr);
	return rules;
}
