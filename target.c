// AUTHORS:
// MYKOLA KUSYY 
// GARRETT MCLAUGHLIN

#include <stdio.h>
#include <sys/stat.h>
#include "target.h"


// will be restructured once I have tested
void setTarget(char* parsedTarg, struct Target* targ){
	targ->target = parsedTarg;
}
void setCommand(char* command, unsigned int index, struct Target* targ){}
void setModTimeTarget(struct Target* targ){
	struct stat filestat;
	if(stat(targ->target, &filestat) < 0){
		targ->modTime = 0; // Has never been created
	}
	targ->modTime = filestat.st_mtime;	
}
void setModTimeChild(struct Target* targ){}
void setChild(unsigned int index, struct Target* targ, struct Target* child){}
