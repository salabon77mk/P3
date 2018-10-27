// AUTHORS
// MYKOLA KUSYY
// GARRETT MCLAUGHLIN
#ifndef TARGET_H
#define TARGET_H

#include <stdlib.h>

struct Target{
	char* target;
	char** commands;
	unsigned int modTime;
	size_t numCommands;
	size_t numChildren;
	struct Target** children;
};

void setTarget(char* parsedTarg, struct Target* targ);
void setCommand(char* command, unsigned int index, struct Target* targ);
void setModTimeTarget(struct Target* targ);
void setModTimeChild(struct Target* targ);
void setChild(unsigned int index, struct Target* targ, struct Target* child);

#endif
