// AUTHORS:
// MYKOLA KUSYY 
// GARRETT MCLAUGHLIN
#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "target.h"
#include "mem_manage.h"

//IMPORTANT NOTE: ALL FREE STATMENTS HAVE BEEN COMMENTED OUT (they follow an exit)
//want to see if CLANG will complain

struct Sizes{
	size_t commandCount;
	size_t childCount;
};

static const size_t MAX_FILE_SIZE = 256; //Linux max file size, 255 reserved for NULL
static const size_t MAX_LINE_SIZE = 1024;

static struct Rules* createRules(struct Target** rules, size_t ruleCount);
static char* parseTarg(FILE *fptr, char* ch, unsigned int* lineNum, char* currLine);
static struct Target** parseChildren(FILE *fptr, char* ch, struct Sizes* sizeCounts, unsigned int* lineNum, char* currLine);
//char*** equiv to string[][], will help for execvp
static char*** parseCommands(FILE *fptr, char* ch, struct Sizes* sizeCounts,  unsigned int* lineNum);

static void skipNewlineComm(FILE *fptr, char* ch,  unsigned int* lineNum );
static void skipWhitespace(FILE *fptr, char* ch);
static void skipWhitespaceTabs(FILE *fptr, char* ch);
static void checkSize(const size_t currLen, const size_t maxLen, unsigned int* lineNum, char* currStr);
static void checkColon(char* ch, unsigned int* lineNum, char* currLine);

static void printBadLine(char* currStr, unsigned int* lineNum);
static char* getWholeLine(FILE *fptr, char* ch, unsigned int* lineNum);


struct Rules* parseRules(FILE *fptr){
	unsigned int lineNum = 0;
	size_t ruleSize = 255;
	struct Target** rules = (struct Target**) mallocWrapper(sizeof(struct Target*), ruleSize);
	size_t ruleCount = 0;
	char ch = fgetc(fptr); 
	while(ch != EOF){
		//Skip blank lines and comments
		skipNewlineComm(fptr, &ch, &lineNum);
		
		if(ruleCount >= ruleSize){
			rules = (struct Target**) doubleAllocatedMem(rules, &ruleSize, sizeof(struct Target*));
		}
		
		//If EOF, just stop
		if(ch != EOF){
			struct Sizes sizeCounts = {0, 0};
		
			char* currLine = getWholeLine(fptr, &ch, &lineNum);
			char* targ = parseTarg(fptr, &ch, &lineNum, currLine);
			struct Target** deps = parseChildren(fptr, &ch, &sizeCounts, &lineNum, currLine);
			currLine = freeAndNULL(currLine);		
			// Handle getting the whole line in parseCommands
			char*** commands = parseCommands(fptr, &ch, &sizeCounts, &lineNum);
			struct Target* rule = createTarget(targ, commands, deps, sizeCounts.commandCount, sizeCounts.childCount);
			rules[ruleCount] = rule;
			ruleCount++;
			printCont(rule);
		}
	}
	//ensure file isn't empty
	if(ruleCount == 0){
		fprintf(stderr, "Empty makefile");
		exit(-1);
	}

	//realloc so rest of program runs nice
	rules = (struct Target**) reallocWrapper(rules, ruleCount, sizeof(struct Target*));
	struct Rules* rulesStruct = createRules(rules, ruleCount);
	return rulesStruct; 
}

static char* parseTarg(FILE *fptr, char* ch, unsigned int* lineNum, char* currLine){
	checkColon(ch, lineNum, currLine);
	skipWhitespace(fptr, ch); //Will make sure we get right to actual content
	size_t counter = 0; //keep track of where we are in the line
	char* str = createStr(MAX_LINE_SIZE);

	while(*ch != EOF && *ch != ':' && counter < MAX_LINE_SIZE){
		str[counter] = *ch;
		counter++;
		*ch = fgetc(fptr);
		if(*ch == ' '){
			skipWhitespace(fptr, ch);
			//if it isn't a colon, that means additional rules -> NOT ALLOWED
			if(*ch != ':'){
				printBadLine(currLine, lineNum);
			}
		}
	}
	if(*ch == EOF){
		printBadLine(currLine, lineNum);
	}

	str[counter] = '\0';
	str = reallocWrapper(str, counter + 1, sizeof(char));
	//realloc;
	*ch = fgetc(fptr); // need to do for future parsing
	return str;
}

static struct Target** parseChildren(FILE *fptr, char* ch, struct Sizes* sizeCounts, unsigned int* lineNum, char* currLine){
	size_t initialChildCount = 255;
	struct Target** deps = (struct Target**) mallocWrapper(sizeof(struct Target*), initialChildCount);
	size_t childCount = 0; //index count for Target** deps
	while(*ch != '\n' && *ch != EOF){
		skipWhitespace(fptr, ch); //skip white space between each child
		//don't check EOF, maybe last deps are all targets
		size_t fileLenCount = 0; //length of file name
		char* str = createStr(MAX_FILE_SIZE);
		/*
		//this wheere we do a realloc
		if(childCount >= initialChildCount){
			deps = (struct Target**) doubleAllocatedMemory(deps, &initialChildCount, sizeof(struct* Target));
		}
		*/
		
		while(*ch != ' ' && fileLenCount < MAX_FILE_SIZE && *ch != EOF && *ch != '\n'){
			checkColon(ch, lineNum, currLine);
			str[fileLenCount] = *ch;
			fileLenCount++;
			*ch = fgetc(fptr);
		}
		//making sure we don't exceeed linux file length
		checkSize(fileLenCount, MAX_FILE_SIZE, lineNum, currLine);	
		// no dependencies, could be something like a clean command
		if(str[0] == '\0') {
			break;
		}
		str[fileLenCount] = '\0';
		struct Target* depen = createChild(str, fileLenCount); //takes care of realloc
		fileLenCount = 0;
		deps[childCount] = depen;
		childCount++;	
	}
	(*lineNum)++; //have to deref first otherwise we're incrementing an address -> BAD
	*ch = fgetc(fptr); //finished with this, move on
	//maybe there's a ton of \n before a command
	skipNewlineComm(fptr, ch, lineNum);
	sizeCounts->childCount = childCount;
	return deps;
}

static char*** parseCommands(FILE *fptr, char* ch, struct Sizes* sizeCounts, unsigned int* lineNum){
	size_t initialCommandCount = 255;
	char*** commands = (char***) calloc(initialCommandCount, sizeof(char **));
	size_t numCommands = 0;

	while(*ch == '\t' && *ch != EOF){

		*ch = fgetc(fptr); //already know it's tab, need to move on
		skipWhitespaceTabs(fptr, ch);
		
		char* wholeLine = getWholeLine(fptr, ch, lineNum); //checks for NULL and max line length 1024
	
		size_t initialLineArgs = 255;	
		commands[numCommands] = (char**) calloc(initialLineArgs, sizeof(char*));	
		size_t currCommand = 0;
		size_t currChar = 0;
		
		while(*ch != '\n' && *ch != EOF){ 
			commands[numCommands][currCommand] = createStr(MAX_LINE_SIZE);
			while((*ch != ' ' && *ch != '\t') && *ch != EOF && *ch != '\n'){
				commands[numCommands][currCommand][currChar] = *ch;
				currChar++;
				*ch = fgetc(fptr);
			}
			commands[numCommands][currCommand][currChar] = '\0';
			//do realloc
			commands[numCommands][currCommand] = 
				(char*) reallocWrapper(commands[numCommands][currCommand], currChar + 1, sizeof(char));
			currChar = 0;
			currCommand++;
			skipWhitespaceTabs(fptr, ch);
		}

		//no commands found after a tab
		if(commands[numCommands][0] == NULL){
		       	printBadLine(wholeLine, lineNum);
		}

		//done adding commands for that line
		commands[numCommands][currCommand] = NULL;
		commands[numCommands] = (char**) reallocWrapper(commands[numCommands], currCommand + 1, sizeof(char*));
		
		if(*ch != EOF){ //Not eof so we can increment line number and get next char
			numCommands++;
			if(numCommands >= initialCommandCount){
				commands = (char***) doubleAllocatedMem(commands, &initialCommandCount, sizeof(char**));
			}
			(*lineNum)++; //going to a new line with new commands
			*ch = fgetc(fptr); 
			skipNewlineComm(fptr, ch, lineNum); // maybe there's a bunch of new lines after eg \t<commands>\n\n\n\n\t<commands>
		}
		wholeLine = freeAndNULL(wholeLine); // Done with current line
	}
	//realloc of char*** handled in target.c
	//don't check EOF here, handled by caller
	sizeCounts->commandCount = numCommands;
	return commands;
}

static struct Rules* createRules(struct Target** parsedRules, size_t ruleCount){
	struct Rules* rules = (struct Rules*) malloc(sizeof(struct Rules));
	if(rules == NULL){
		fprintf(stderr, "Failed to malloc in parser.c:createRules");
		exit(-1);
	}

	rules->numRules = ruleCount;
	rules->rules = parsedRules;

	return rules;
}

static void skipNewlineComm(FILE *fptr, char* ch, unsigned int* lineNum){ 
	while((*ch == '\n' || *ch == '#') && *ch != EOF){
		if(*ch == '\n'){
			while((*ch = fgetc(fptr)) != EOF && *ch == '\n'){
				(*lineNum)++;
			}			
		}
		else if(*ch == '#'){
			char* currLine = getWholeLine(fptr, ch, lineNum); //checks for lengths and null bytes
			currLine = freeAndNULL(currLine); //safe to parse ahead
			while((*ch = fgetc(fptr)) != EOF && *ch != '\n');
			if(*ch != EOF){
				*ch = fgetc(fptr);
				(*lineNum)++;
			}
		}
	}
}

static void skipWhitespace(FILE *fptr, char* ch){
	if(*ch == ' ' && *ch != EOF){
		while(( *ch = fgetc(fptr)) != EOF && *ch == ' ');
	}
}

static void skipWhitespaceTabs(FILE *fptr, char* ch){
	if((*ch == ' ' || *ch == '\t') && *ch != EOF){
		while((*ch = fgetc(fptr)) != EOF && (*ch == ' ' || *ch == '\t'));
	}
}

static void checkSize(const size_t currLen, const size_t maxLen, unsigned int* lineNum, char* currStr){
	if(currLen >= maxLen){
		fprintf(stderr, "Exceeded size at line %u: %s\n", *lineNum, currStr);
		exit(-1);
	}
}

static void checkColon(char* ch, unsigned int* lineNum, char* currLine){
	if(*ch == ':'){
		printBadLine(currLine, lineNum);
	}
}

static void printBadLine(char* currStr, unsigned int* lineNum){
	fprintf(stderr, "%u: Invalid Line: %s\n", *lineNum, currStr);
	exit(-1);
}

static char* getWholeLine(FILE *fptr, char* ch, unsigned int* lineNum){
	fpos_t position;
	fgetpos(fptr, &position);
	char pbvChar = *ch;
	size_t counter = 0;
	char* wholeLine = createStr(MAX_LINE_SIZE);

	while(pbvChar != EOF && pbvChar != '\n' && counter < MAX_LINE_SIZE){
		//ensure there's no NULL byte in the middle of the string
		if(pbvChar == '\0'){ 
			printBadLine(wholeLine, lineNum);
			exit(-1);
		}
		wholeLine[counter] = pbvChar;
		counter++;
		pbvChar = fgetc(fptr);	
	}
	checkSize(counter, MAX_LINE_SIZE, lineNum, wholeLine);
	fsetpos(fptr, &position); //reset file pointer back to beginning of line before exiting
	return wholeLine;
}

