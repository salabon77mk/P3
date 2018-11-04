// AUTHORS:
// MYKOLA KUSYY 
// GARRETT MCLAUGHLIN
#include <stdlib.h>

#ifndef CMD_PARSE_H
#define CMD_PARSE_H
struct parsedCMD {
	char* specifiedMakefile;
	char* specifiedRule;
	size_t makefileSize;
	int fFlag;
};

struct parsedCMD* parseCMD(int argc, char** argv);
#endif
