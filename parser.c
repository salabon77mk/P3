// AUTHORS:
// MYKOLA KUSYY 
// GARRETT MCLAUGHLIN
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "target.h"

struct Sizes{
	size_t childCount;
	size_t commandCount;
};

static const size_t MAX_CHILDREN_SIZE = 255;
static const size_t MAX_FILE_SIZE = 255; //Linux max file size
static const size_t MAX_LINE_SIZE = 1024;


//all of these I think need a line number passed through into them
static char* parseTarg(FILE *fptr, char* ch);
static struct Target** parseChildren(FILE *fptr, char* ch, struct Sizes* sizeCounts);
static char** parseCommands(FILE *fptr, char* ch, struct Sizes* sizeCounts);
static void skipNewline(FILE *fptr, char* ch /*,  unsigned int* lineNum*/);
static void skipWhitespace(FILE *fptr, char* ch);
static char* createStr(size_t size);
static void checkEOF(FILE *fptr, char* ch);

struct Target** parseRules(FILE *fptr){
	unsigned int lineNum = 0;
	
	struct Target** rules = (struct Target**) malloc(sizeof(struct Target*) * MAX_CHILDREN_SIZE);
	size_t ruleCount = 0;
	char ch; 
	while((ch = fgetc(fptr))!= EOF){
		//Skip blank lines
		skipNewline(fptr, &ch);
		struct Sizes sizeCounts = {0, 0};
		
		char* targ = parseTarg(fptr, &ch);
		struct Target** deps = parseChildren(fptr, &ch, &sizeCounts);
		char** commands = parseCommands(fptr, &ch, &sizeCounts);
		struct Target* rule = createTarget(targ, commands, deps, sizeCounts.commandCount, sizeCounts.childCount);
		rules[ruleCount] = rule;
		ruleCount++;
		printCont(rule);
	}

	//realloc rules to proper size
	return NULL; //temporary
}

static char* parseTarg(FILE *fptr, char* ch){

	if(*ch == ':'){
		//TODO INCORPORATE LINE NUMBER IN ERROR
		fprintf(stderr, "Target started with :, exiting");
		exit(-1);
	}

	char* str = createStr(MAX_FILE_SIZE);
	size_t counter = 0;
	//TODO Exit if EOF encountered in such a potential line
	while(*ch != EOF){
		if(counter >= MAX_FILE_SIZE){
			//TODO TOO BIG PRINT TO STDERR WITH LINE NUM
		}

		if(counter < MAX_FILE_SIZE && *ch != ':'){
			str[counter] = *ch;
			counter++;
			*ch = fgetc(fptr);
		}

		else if(counter < MAX_FILE_SIZE && *ch == ':'){
			str[counter] = '\0';
//			*ch = fgetc(fptr); // need to do for future parsing
			return str;
		}
	}
	free(str); //if we don't, clang will get mad
	checkEOF(fptr, ch); //TODO need to flesh out because make should terminate if EOF encountered after a target
	//TODO HANDLE EOF
	return NULL; //probably should be something else
}

static struct Target** parseChildren(FILE *fptr, char* ch, struct Sizes* sizeCounts){
	struct Target** deps = (struct Target**) malloc(sizeof(struct Target) * MAX_CHILDREN_SIZE);
	
	size_t childCount = 0;
	size_t lineLenCount = 0; //length of the string
	while((*ch = fgetc(fptr)) != '\n' && lineLenCount < MAX_LINE_SIZE && *ch != EOF){
		skipWhitespace(fptr, ch);	

		size_t fileLenCount = 0; //length of file name
		char* str = createStr(MAX_FILE_SIZE);
		
		while(*ch != ' ' && fileLenCount < MAX_FILE_SIZE && *ch != EOF && *ch != '\n'){
			str[fileLenCount] = *ch;
			fileLenCount++;
			lineLenCount++; //we're still on a line that must be incremented
			*ch = fgetc(fptr);
		}
		// Add null char, no need to increment where we are in the line
		if(fileLenCount < MAX_FILE_SIZE && childCount < MAX_CHILDREN_SIZE){
			str[fileLenCount] = '\0';
			fileLenCount = 0;
			struct Target* depen = createChild(str);
			deps[childCount] = depen;
			childCount++;
		}

		else{
			//TODO STRING TOO LONG OR RAN OUT OF SPACE FOR CHILDREN
			free(str);
		}
	}
	 //for future parsing
	checkEOF(fptr, ch); 
	sizeCounts->childCount = childCount;
	return deps;
}

static char** parseCommands(FILE *fptr, char* ch, struct Sizes* sizeCounts){
	char** commands = (char**) malloc(sizeof(char *) * MAX_CHILDREN_SIZE);
	size_t numCommands = 0;
	while(*ch == '\t' && *ch != EOF){
		
		size_t currLineLen = 0;
		char* str = createStr(MAX_LINE_SIZE);

		while((*ch = fgetc(fptr)) != EOF && currLineLen < MAX_LINE_SIZE && *ch != '\n'){
			str[currLineLen] = *ch;
			currLineLen++;
		}
		if(currLineLen < MAX_LINE_SIZE && numCommands < MAX_CHILDREN_SIZE){
			str[currLineLen] = '\0';
			currLineLen = 0;
			commands[numCommands] = str;
			numCommands++;
			*ch = fgetc(fptr); 
		}

		else{
			//TODO TOO LONG
			free(str);
		}
	}
	sizeCounts->commandCount = numCommands;
	return commands;
}

static void skipNewline(FILE *fptr, char* ch){ 
	if(*ch == '\n'){
		while(( *ch = fgetc(fptr)) != EOF && *ch == '\n');			
	}
}

static void skipWhitespace(FILE *fptr, char* ch){
	if(*ch == ' '){
		while(( *ch = fgetc(fptr)) != EOF && *ch == ' ');
	}
}

static char* createStr(size_t size){
	char* str = (char *) malloc(sizeof(char) * size);		
	if(str == NULL){
		fprintf(stderr, "malloc failed");
		exit(-1);
	}
	return str;
}

static void checkEOF(FILE *fptr, char* ch){
	if(*ch == EOF){
		fprintf(stderr, "Encountered EOF after either a rule or target");
		exit(-1);
	}
	*ch = fgetc(fptr);
}
