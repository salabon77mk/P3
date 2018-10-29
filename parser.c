// AUTHORS:
// MYKOLA KUSYY 
// GARRETT MCLAUGHLIN
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "target.h"

//IMPORTANT NOTE: ALL FREE STATMENTS HAVE BEEN COMMENTED OUT (they follow an exit)
//want to see if CLANG will complain

struct Sizes{
	size_t commandCount;
	size_t childCount;
};

static const size_t MAX_CHILDREN_SIZE = 255;
static const size_t MAX_FILE_SIZE = 255; //Linux max file size
static const size_t MAX_LINE_SIZE = 1024;


static char* parseTarg(FILE *fptr, char* ch,  unsigned int* lineNum);
static struct Target** parseChildren(FILE *fptr, char* ch, struct Sizes* sizeCounts,  unsigned int* lineNum);
static char** parseCommands(FILE *fptr, char* ch, struct Sizes* sizeCounts,  unsigned int* lineNum);
static void skipNewline(FILE *fptr, char* ch,  unsigned int* lineNum );
static void skipWhitespace(FILE *fptr, char* ch);
static char* createStr(size_t size);
static void checkEOF(char* ch);
static struct Rules* createRules(struct Target** rules, size_t ruleCount);


struct Rules* parseRules(FILE *fptr){
	unsigned int lineNum = 0;
	
	struct Target** rules = (struct Target**) malloc(sizeof(struct Target*) * MAX_CHILDREN_SIZE);

	if( rules == NULL){
		fprintf(stderr, "malloc failed in parse rules");
		exit(-1);
	}
	size_t ruleCount = 0;
	char ch = fgetc(fptr); 
	while(ch != EOF && ruleCount < MAX_CHILDREN_SIZE){
		//Skip blank lines
		skipNewline(fptr, &ch, &lineNum);
		
		//If EOF, just stop
		if( ch != EOF){
			//check if file is just a bunch of \n chars
			struct Sizes sizeCounts = {0, 0};
		
			char* targ = parseTarg(fptr, &ch, &lineNum);
			struct Target** deps = parseChildren(fptr, &ch, &sizeCounts, &lineNum);
			char** commands = parseCommands(fptr, &ch, &sizeCounts, &lineNum);
			struct Target* rule = createTarget(targ, commands, deps, sizeCounts.commandCount, sizeCounts.childCount);
			rules[ruleCount] = rule;
			ruleCount++;
			printCont(rule);
		}
	}
	//ensure file isn't empty
	if((ruleCount) == 0){
		fprintf(stderr, "Empty makefile");
		exit(-1);
	}

	//exit if too many children
	if((ruleCount) >= MAX_CHILDREN_SIZE){
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

static char* parseTarg(FILE *fptr, char* ch, unsigned int* lineNum){

	if(*ch == ':'){
		//TODO INCORPORATE LINE NUMBER IN ERROR
		fprintf(stderr, "Target started with : at lineNum %u\n", *lineNum);
		exit(-1);
	}

	size_t counter = 0; //keep track of where we are in the line
	char* str = createStr(MAX_FILE_SIZE);
	skipWhitespace(fptr, ch); //Will make sure we get right to actual content, also checks eof eg get a line "               EOF"
		

	while(*ch != EOF && *ch != ':' && counter < MAX_FILE_SIZE){
		str[counter] = *ch;
		counter++;
		*ch = fgetc(fptr);
	}

	checkEOF(ch); //Accounts for a line that could be "target     EOF"
	if(counter >= MAX_FILE_SIZE){
	//	free(str);
		fprintf(stderr, "Exceeded file length at line number %u\n", *lineNum);
		exit(-1);
	}

	str[counter] = '\0';
	*ch = fgetc(fptr); // need to do for future parsing
	return str;
	
}

static struct Target** parseChildren(FILE *fptr, char* ch, struct Sizes* sizeCounts, unsigned int* lineNum){
	struct Target** deps = (struct Target**) malloc(sizeof(struct Target) * MAX_CHILDREN_SIZE);
	if(deps == NULL){
		fprintf(stderr, "Malloc failed in parse children");
		exit(-1);
	}

	size_t childCount = 0; //index count for Target** deps
	size_t lineLenCount = 0; //length of the string
	while(*ch != '\n' && lineLenCount < MAX_LINE_SIZE && *ch != EOF){
		skipWhitespace(fptr, ch); //skip white space between each child and checks for unexpected EOF eg "target: rule1 rule2 EOF"	

		size_t fileLenCount = 0; //length of file name
		char* str = createStr(MAX_FILE_SIZE);
		
		while(*ch != ' ' && fileLenCount < MAX_FILE_SIZE && *ch != EOF && *ch != '\n'){
			str[fileLenCount] = *ch;
			fileLenCount++;
			lineLenCount++; //we're still on a line that must be incremented
			*ch = fgetc(fptr);
		}
		if(fileLenCount >= MAX_FILE_SIZE){
//			free(str);
			fprintf(stderr,"Too long file length at line number %u", *lineNum);	
			exit(-1);	
		}
		checkEOF(ch); //safety check for next conditionals

		// Scanned the line but no children found
		if(childCount == 0 && *ch == '\n'){
			free(str);
			fprintf(stderr, "No commands given at line number %u", *lineNum);
		}
		if(childCount >= MAX_CHILDREN_SIZE){
			fprintf(stderr, "Too many files included");
			exit(-1);
		}
		
		//passed all checks, proceed
		// Add null char, no need to increment where we are in the line
		str[fileLenCount] = '\0';
		fileLenCount = 0;
		struct Target* depen = createChild(str);
		deps[childCount] = depen;
		childCount++;
		
	}
	(*lineNum)++; //have to deref first otherwise we're incrementing an address -> BAD
	checkEOF(ch); 
	*ch = fgetc(fptr); //finished with this, move on
	//maybe there's a ton of \n before a command
	skipNewline(fptr, ch, lineNum);
	checkEOF(ch); //check after in case we reached EOF eg "target: dep1 dep2 \n\n\n\n\n\n\nEOF"
	sizeCounts->childCount = childCount;
	return deps;
}

static char** parseCommands(FILE *fptr, char* ch, struct Sizes* sizeCounts, unsigned int* lineNum){
	char** commands = (char**) malloc(sizeof(char *) * MAX_CHILDREN_SIZE);
	size_t numCommands = 0;
	while(*ch == '\t' && *ch != EOF){
		
		skipWhitespace(fptr, ch); //in case we get something like \t             EOF
		size_t currLineLen = 0;
		char* str = createStr(MAX_LINE_SIZE);
		
		//parse the line. using fgets because first char will be \t
		while((*ch = fgetc(fptr)) != EOF && currLineLen < MAX_LINE_SIZE && *ch != '\n'){
			str[currLineLen] = *ch;
			currLineLen++;
		}

		//too long
		if(currLineLen >= MAX_LINE_SIZE){
		//	free(str);
			fprintf(stderr, "Exceeded line length at line number %u", *lineNum);
			exit(-1);
		}

		//conditional if numCommands is zero but encountered new line?
		if(numCommands == 0 && *ch == '\t'){
			free(str);
			fprintf(stderr, "No commands found for that recipe at line number %u", *lineNum);
			exit(-1);
		}
		

		str[currLineLen] = '\0';
		currLineLen = 0;
		commands[numCommands] = str;
		numCommands++;
		(*lineNum)++; //going to a new line with new commands
		*ch = fgetc(fptr); 
		skipNewline(fptr, ch, lineNum); // maybe there's a bunch of new lines after eg \t<commands>\n\n\n\n\t<commands>
	}
	//don't check EOF here, handled by caller
	sizeCounts->commandCount = numCommands;
	return commands;
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

	checkEOF(ch); 
}

static char* createStr(size_t size){
	char* str = (char *) malloc(sizeof(char) * size);		
	if(str == NULL){
		fprintf(stderr, "malloc failed");
		exit(-1);
	}
	return str;
}
//peek ahead to see if unexpected EOF
static void checkEOF(char* ch){
	if(*ch == EOF){
		fprintf(stderr, "Encountered EOF after either a rule or target");
		exit(-1);
	}
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
