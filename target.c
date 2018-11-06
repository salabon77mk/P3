// AUTHORS:
// MYKOLA KUSYY 
// GARRETT MCLAUGHLIN

#include <stdio.h>
#include <sys/stat.h>
#include "target.h"
#include "parser.h"
#include "mem_manage.h"

// will be restructured once I have tested

static int setModTime(struct Target* targ);
static size_t* getNumArgs(char*** commands, size_t numComms);


//struct Target* createTarget(char* fileName, char*** comms, struct Target** deps, size_t numComms, size_t numChild){
struct Target* createTarget(char* fileName, char*** comms, struct Target** deps, struct Sizes* sizePOD){
	struct Target* targ = (struct Target*) malloc(sizeof(struct Target));
	if(targ == NULL){
		fprintf(stderr, "malloc failed");
		exit(-1);
	}
	//realloc the arrays so it fits properly
	if(sizePOD->commandCount > 0){
		comms = (char***) reallocWrapper(comms, sizePOD->commandCount, sizeof(char**));
		targ->numArgs = getNumArgs(comms, sizePOD->commandCount);
	}
	else{
		targ->numArgs = NULL;
	}

	if(sizePOD->childCount > 0){
		deps = (struct Target**) reallocWrapper(deps, sizePOD->childCount, sizeof(struct Target*));
	}

	targ->target = fileName;
	targ->commands = comms;
	targ->children = deps;

	targ->targetLen = sizePOD->targetLen; 
	targ->numCommands = sizePOD->commandCount;
	targ->numChildren = sizePOD->childCount;

	targ->modTime = setModTime(targ);
	targ->isRule = 1; // 0 is our false value
	targ->visited = 0; // for checking circular deps later
	return targ;
}

struct Target* createChild(char* fileName, size_t fileLen){
	struct Target* targ = (struct Target*) malloc(sizeof(struct Target));

	if(targ == NULL){
		fprintf(stderr, "malloc failed");
		exit(-1);
	}
	
	
	fileName = (char*) reallocWrapper(fileName, fileLen, sizeof(char));
	targ->target = fileName;
		
	targ->commands = NULL;
	targ->children = NULL;
	targ->numArgs = NULL;
	targ->numCommands = 0;
	targ->numChildren = 0;
	targ->modTime = setModTime(targ);
	targ->targetLen = fileLen;
	targ->isRule = 0; //not a rule
	targ->visited = -1; // we don't check circular dependency if not a rule
	
	return targ;
}

static int setModTime(struct Target* targ){
	struct stat filestat;
	if(stat(targ->target, &filestat) < 0){
		return 0/*7fffffff*/; // Has never been created
	}
	return filestat.st_mtime;	
}

static size_t* getNumArgs(char*** commands, size_t numComms){
	size_t* numArgs = mallocWrapper(numComms, sizeof(size_t));
	for(size_t i = 0; i < numComms; i++){
		size_t currArgCount = 0;
		int j = 0;
		while(commands[i][j] != '\0'){
			currArgCount++;
			j++;
		}
		numArgs[i] = currArgCount;
	}
	return numArgs;
}


void printCont(const struct Target* targ){
	printf("FILENAME %s\n", targ->target);
	printf("numCommands %zu\n", targ->numCommands);
	printf("numChildren %zu\n", targ->numChildren);
	
	for(size_t i = 0; i < targ->numCommands; i++){
//		int j = 0;
		for(size_t j = 0; j < targ->numArgs[i]; j++){
			printf("COMMANDS %s\n", targ->commands[i][j]);
		}

		/*
		while(targ->commands[i][j] != '\0'){
			j++;
		}
		*/
		printf("NUMARGS for command #%zu: %zu \n", targ->numCommands, targ->numArgs[i]);
	}

	for(size_t i = 0; i < targ->numChildren; i++){
		printf("TARGET CHILD #%zu %s \n", i, targ->children[i]->target);
	}
	printf("\n");

}


