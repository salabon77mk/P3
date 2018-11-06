// AUTHORS
// MYKOLA KUSYY
// GARRETT MCLAUGHLIN
#ifndef TARGET_H
#define TARGET_H

#include <stdlib.h>
#include "sizes_struct.h"

struct Target{
	char* target;
	char*** commands;
	time_t modTime;
	int isRule;
	int visited;
	//isFile
	size_t targetLen;
	size_t numCommands;
	size_t numChildren;
	size_t* numArgs;
	struct Target** children;
};


struct Target* createTarget(char* fileName, char*** comms, struct Target** deps, struct Sizes* sizePOD);

struct Target* createChild(char* fileName, size_t targetLen);

void printCont(const struct Target* targ);
#endif
