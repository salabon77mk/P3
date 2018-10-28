// AUTHORS
// MYKOLA KUSYY
// GARRETT MCLAUGHLIN
#ifndef TARGET_H
#define TARGET_H

#include <stdlib.h>

struct Target{
	char* target;
	char** commands;
	int modTime;
	int isRule;
	//int isFile // modify this in setMoDTime?
	size_t numCommands;
	size_t numChildren;
	struct Target** children;
};


struct Target* createTarget(char* fileName, char** comms, struct Target** deps, size_t numComms, size_t numChild);

struct Target* createChild(char* fileName);

void printCont(const struct Target* targ);
#endif
