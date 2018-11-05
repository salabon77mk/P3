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

struct Target* createTarget(char* fileName, char*** comms, struct Target** deps, size_t numComms, size_t numChild){
	struct Target* targ = (struct Target*) malloc(sizeof(struct Target));
	if(targ == NULL){
		fprintf(stderr, "malloc failed");
		exit(-1);
	}

	//realloc the arrays so it fits properly
	if(numComms > 0){
		comms = (char***) reallocWrapper(comms, numComms, sizeof(char**));
	}

	if(numChild > 0){
		deps = (struct Target**) reallocWrapper(deps, numChild, sizeof(struct Target*));
	}
	targ->target = fileName;
	targ->commands = comms;
	targ->children = deps;
	targ->numCommands = numComms;
	targ->numChildren = numChild;
	targ->modTime = setModTime(targ);
	targ->targetLen = 0; 
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

	char* targName = (char*) realloc(fileName, fileLen * sizeof(char));
	if(targName == NULL){
		free(targName);
		fprintf(stderr, "Failed to realloc in Target");
		exit(-1);
	}

	targ->target = fileName;
	targ->commands = NULL;
	targ->children = NULL;
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


void printCont(const struct Target* targ){
	printf("FILENAME %s\n", targ->target);
	printf("numCommands %zu\n", targ->numCommands);
	printf("numChildren %zu\n", targ->numChildren);
	
	for(size_t i = 0; i < targ->numCommands; i++){
		int j = 0;
		while(targ->commands[i][j] != NULL){
			printf("COMMANDS %s\n", targ->commands[i][j]);
			j++;
		}
	}

	for(size_t i = 0; i < targ->numChildren; i++){
		printf("TARGET CHILD #%zu %s \n", i, targ->children[i]->target);
	}
	printf("\n");

}
