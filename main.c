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
#include "cmd_parse.h"

static char* getMake(char* specifiedMake, size_t makeLength);
static struct Rules* getRules(const char* makefile);


int main(int argc, char** argv){
	struct parsedCMD* args = parseCMD(argc, argv);
	const char* makefile = getMake(args->specifiedMakefile, args->makefileSize);
	struct Rules* rules = getRules(makefile);
	rules = createGraph(rules);	
	if(args->specifiedRule != NULL){
		struct Target* desiredRule = getRule(rules, args->specifiedRule); 
		if(desiredRule != NULL){
			build(desiredRule);
		}
		else{
			fprintf(stderr, "Specified rule: %s not found, exiting", args->specifiedRule);
			exit(-1);
		}
	}
	else{
		build(rules->rules[0]);
	}

	printf("Stop here debugger!");
}

// Find a makefile
// If it exists, we will be able to safely open it and pass it to parser
static char* getMake(char* specifiedMake, size_t makeLength){
	//TODO Should be easy to implement extra credit with an arg here
	DIR *dirp = opendir(".");
	struct dirent *entry;

	
	char* lilMake = "testmake"; //CHANGE TO makefile WHEN FINISHED
	char* bigMake = "Makefile";
	const size_t defaultMakeSize = 8; // 
	//extra credit const char* targetFile

	while((entry = readdir(dirp)) != NULL){
		if(specifiedMake == NULL){
			if(!strncmp(entry->d_name, lilMake, defaultMakeSize)){
				closedir(dirp);
				return lilMake;
		}

			if(!strncmp(entry->d_name, bigMake, defaultMakeSize)){
				closedir(dirp);
				return bigMake;
			}
		}
		else{
			if(!strncmp(entry->d_name, specifiedMake, makeLength)){
				closedir(dirp);
				return specifiedMake;
			}
		}
	}


	closedir(dirp);
	fprintf(stderr, "No makefile found, exiting \n");
	exit(-1);
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
