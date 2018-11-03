// AUTHORS:
// MYKOLA KUSYY 
// GARRETT MCLAUGHLIN
#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "target.h"

//IMPORTANT NOTE: ALL FREE STATMENTS HAVE BEEN COMMENTED OUT (they follow an exit)
//want to see if CLANG will complain

struct Sizes{
	size_t commandCount;
	size_t childCount;
};

static const size_t MAX_FILE_SIZE = 255; //Linux max file size
static const size_t MAX_LINE_SIZE = 1024;

static struct Rules* createRules(struct Target** rules, size_t ruleCount);
static char* parseTarg(FILE *fptr, char* ch, unsigned int* lineNum, char* currLine);
static struct Target** parseChildren(FILE *fptr, char* ch, struct Sizes* sizeCounts, unsigned int* lineNum, char* currLine);
static char*** parseCommands(FILE *fptr, char* ch, struct Sizes* sizeCounts,  unsigned int* lineNum);

static void skipNewline(FILE *fptr, char* ch,  unsigned int* lineNum );
static void skipWhitespace(FILE *fptr, char* ch);
static void checkSize(const size_t currLen, const size_t maxLen, unsigned int* lineNum, char* currStr);
static void checkEOF(char* ch, unsigned int* lineNum);
static void checkColon(char* ch, unsigned int* lineNum, char* currLine);
static void checkNULL(char* ch, unsigned int* lineNum);

static char* createStr(size_t size);
static void* reallocCheck(void* data, size_t currSize, size_t typeSize);
static void* mallocCheck(void* somePointer);
static void* freeAndNULL(void* ptr);

static void printBadLine(char* currStr, unsigned int* lineNum);
static char* getWholeLine(FILE *fptr, char* ch, unsigned int* lineNum);


struct Rules* parseRules(FILE *fptr){
	unsigned int lineNum = 0;
	
	struct Target** rules = (struct Target**) malloc(sizeof(struct Target*) * MAX_FILE_SIZE);

	if( rules == NULL){
		fprintf(stderr, "malloc failed in parse rules");
		exit(-1);
	}
	size_t ruleCount = 0;
	char ch = fgetc(fptr); 
	while(ch != EOF && ruleCount < MAX_FILE_SIZE){
		//Skip blank lines
		skipNewline(fptr, &ch, &lineNum); // skipping blank lines

		//If EOF, just stop
		if( ch != EOF){
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

	//exit if too many children //add this check inside loop above
	if(ruleCount >= MAX_FILE_SIZE){
		fprintf(stderr, "File contains too many rules");
		exit(-1);
	}

	//realloc so rest of program will work nicely
	struct Target** returnRules = (struct Target**) realloc(rules, ruleCount * sizeof(struct Target*));
	if(returnRules == NULL){
		free(returnRules);
		fprintf(stderr, "Failed to realloc in parseRules");
		exit(-1);
	}

	struct Rules* rulesStruct = createRules(rules, ruleCount);
	return rulesStruct; 
}

static char* parseTarg(FILE *fptr, char* ch, unsigned int* lineNum, char* currLine){
	checkColon(ch, lineNum, currLine);
	skipWhitespace(fptr, ch); //Will make sure we get right to actual content, also checks eof eg get a line "               EOF"

	size_t counter = 0; //keep track of where we are in the line
	char* str = createStr(MAX_FILE_SIZE);

	while(*ch != EOF && *ch != ':' && counter < MAX_FILE_SIZE){
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

	checkEOF(ch, lineNum); //Accounts for a line that could be "target     EOF"
//	checkSize(counter, MAX_FILE_SIZE, lineNum);

	str[counter] = '\0';
	*ch = fgetc(fptr); // need to do for future parsing
	return str;
	
}

static struct Target** parseChildren(FILE *fptr, char* ch, struct Sizes* sizeCounts, unsigned int* lineNum, char* currLine){
	struct Target** deps = (struct Target**) malloc(sizeof(struct Target) * MAX_FILE_SIZE);
	if(deps == NULL){
		fprintf(stderr, "Malloc failed in parse children");
		exit(-1);
	}

	size_t childCount = 0; //index count for Target** deps
	size_t lineLenCount = 0; //length of the string
	while(*ch != '\n' && lineLenCount < MAX_LINE_SIZE){
		skipWhitespace(fptr, ch); //skip white space between each child and checks for unexpected EOF eg "target: rule1 rule2 EOF"
		checkEOF(ch, lineNum);
		size_t fileLenCount = 0; //length of file name
		char* str = createStr(MAX_FILE_SIZE);
		
		while(*ch != ' ' && fileLenCount < MAX_FILE_SIZE && *ch != EOF && *ch != '\n'){
			str[fileLenCount] = *ch;
			fileLenCount++;
			lineLenCount++; //we're still on a line that must be incremented
			*ch = fgetc(fptr);
		}
//		checkSize(fileLenCount, MAX_FILE_SIZE, lineNum);
		checkEOF(ch, lineNum); //safety check for next conditionals
		
		/*
		//this wheere we do a realloc
		if(childCount >= MAX_FILE_SIZE){
			fprintf(stderr, "Too many files included");
			exit(-1);
		}
		*/
		str[fileLenCount] = '\0';
		struct Target* depen = createChild(str, fileLenCount);
		fileLenCount = 0;
		deps[childCount] = depen;
		childCount++;
		
	}
	(*lineNum)++; //have to deref first otherwise we're incrementing an address -> BAD
	checkEOF(ch, lineNum); 
	*ch = fgetc(fptr); //finished with this, move on
	//maybe there's a ton of \n before a command
	skipNewline(fptr, ch, lineNum);
	checkEOF(ch, lineNum); //check after in case we reached EOF eg "target: dep1 dep2 \n\n\n\n\n\n\nEOF"
	sizeCounts->childCount = childCount;
	return deps;
}

static char*** parseCommands(FILE *fptr, char* ch, struct Sizes* sizeCounts, unsigned int* lineNum){
	char*** commands = (char***) calloc(MAX_FILE_SIZE, sizeof(char **));
	size_t numCommands = 0;
	while(*ch == '\t' && *ch != EOF){
		*ch = fgetc(fptr); //already know it's tab, need to move on
		skipWhitespace(fptr, ch); 
		checkEOF(ch, lineNum); //in case we get something like \t             EOF
		
		size_t currCommand = 0;
		size_t currLineLen = 0;
		size_t currChar = 0;
		commands[numCommands] = (char**) calloc(MAX_FILE_SIZE, sizeof(char*));	
		
		while(currLineLen < MAX_LINE_SIZE && *ch != '\n' && *ch != EOF){ 
			commands[numCommands][currCommand] = createStr(MAX_LINE_SIZE);
			while(*ch != ' ' && *ch != EOF && currLineLen < MAX_LINE_SIZE && *ch != '\n'){
				commands[numCommands][currCommand][currChar] = *ch;
				currChar++;
				currLineLen++;
				*ch = fgetc(fptr);
			}
//			checkSize(currLineLen, MAX_LINE_SIZE, lineNum);	

			commands[numCommands][currCommand][currChar] = '\0';
			currChar = 0;
			currCommand++;
			skipWhitespace(fptr, ch);
		}
		//conditional if numCommands is zero but encountered new line?
		/*
		if(numCommands == 0 && *ch == '\t'){
			free(str);
			fprintf(stderr, "No commands found for that recipe at line number %u", *lineNum);
			exit(-1);
		}
		*/
		if(*ch != EOF){
//			checkSize(currLineLen, MAX_LINE_SIZE, lineNum);		
			currLineLen = 0;
			numCommands++;
			(*lineNum)++; //going to a new line with new commands
			*ch = fgetc(fptr); 
			skipNewline(fptr, ch, lineNum); // maybe there's a bunch of new lines after eg \t<commands>\n\n\n\n\t<commands>
		}
	}
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

static void skipNewline(FILE *fptr, char* ch, unsigned int* lineNum){ 
	if(*ch == '\n' && *ch != EOF){
		while(( *ch = fgetc(fptr)) != EOF && *ch == '\n'){
			(*lineNum)++;
		}			
	}

}

static void skipWhitespace(FILE *fptr, char* ch){
	if(*ch == ' ' && *ch != EOF){
		while(( *ch = fgetc(fptr)) != EOF && *ch == ' ');
	}

}

//peek ahead to see if unexpected EOF
static void checkEOF(char* ch, unsigned int* lineNum){
	if(*ch == EOF){
		fprintf(stderr, "Unexpected EOF in line:%u", *lineNum);
		exit(-1);
	}
}

static void checkSize(const size_t currLen, const size_t maxLen, unsigned int* lineNum, char* currStr){
	if(currLen >= maxLen){
		fprintf(stderr, "Exceeded line length at line %u: %s", *lineNum, currStr);
		exit(-1);
	}
}

static void checkColon(char* ch, unsigned int* lineNum, char* currLine){
	if(*ch == ':'){
		printBadLine(currLine, lineNum);
	}
}

static void checkNULL(char* ch, unsigned int* lineNum){
	if(*ch == '\0'){
		fprintf(stderr, "Unexpected NULL byte in line number: %u", *lineNum);
		exit(-1);
	}
}

static char* createStr(size_t size){
	char* str = (char *) calloc(size,  sizeof(char));		
	if(str == NULL){
		fprintf(stderr, "malloc failed");
		exit(-1);
	}
	return str;
}

static void* reallocCheck(void* data, size_t currSize, size_t typeSize){
	void* newData =  realloc(data, (currSize * 2) * sizeof(typeSize));
	if( newData == NULL){
		free(newData);
		fprintf(stderr, "Failed to realloc in parser.c:reallocCheck");
		exit(-1);
	}
	return newData;
}
static void* mallocWrapper(size_t dataType, size_t multiplier){
	void* data = malloc(dataType * multiplier);
	if(data == NULL){
		printf("allocation failed, exiting");
		exit(-1);
	}
	return data;
}

static void printBadLine(char* currStr, unsigned int* lineNum){
	fprintf(stderr, "%u: Invalid Line: %s", *lineNum, currStr);
	exit(-1);
}

static char* getWholeLine(FILE *fptr, char* ch, unsigned int* lineNum){
	fpos_t position;
	fgetpos(fptr, &position);
	char pbvChar = *ch;
	size_t counter = 0;
	char* wholeLine = createStr(MAX_LINE_SIZE);

	while(pbvChar != EOF && pbvChar != '\n' && counter < MAX_LINE_SIZE){
		checkNULL(&pbvChar, lineNum);
		wholeLine[counter] = pbvChar;
		counter++;
		pbvChar = fgetc(fptr);	
	}
	checkSize(counter, MAX_LINE_SIZE, lineNum, wholeLine);
	fsetpos(fptr, &position); //reset file pointer back to beginning of line before exiting
	return wholeLine;
}

static void* freeAndNULL(void* ptr){
	free(ptr);
	ptr = NULL;
	return ptr;
}
