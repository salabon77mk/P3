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


//all of these I think need a line number passed through into them
static char* parseTarg(FILE *fptr, char* ch);
static struct Target** parseChildren(FILE *fptr, char* ch, struct Sizes* sizeCounts);
static char** parseCommands(FILE *fptr, char* ch, struct Sizes* sizeCounts);
static void skipNewline(FILE *fptr, char* ch /*,  unsigned int* lineNum*/);
static void skipWhitespace(FILE *fptr, char* ch);
static char* createStr(size_t size);
static void checkEOF(char* ch);

struct Target** parseRules(FILE *fptr){
	unsigned int lineNum = 0;
	
	struct Target** rules = (struct Target**) malloc(sizeof(struct Target*) * MAX_CHILDREN_SIZE);

	if( rules == NULL){
		fprintf(stderr, "malloc failed in parse rules");
		exit(-1);
	}
	size_t ruleCount = 0;
	char ch = fgetc(fptr); 
	while(ch != EOF){
		//Skip blank lines
		skipNewline(fptr, &ch);
		
		if( ch != EOF){
			//check if file is just a bunch of \n chars
			struct Sizes sizeCounts = {0, 0};
		
			char* targ = parseTarg(fptr, &ch);
			struct Target** deps = parseChildren(fptr, &ch, &sizeCounts);
			char** commands = parseCommands(fptr, &ch, &sizeCounts);
			struct Target* rule = createTarget(targ, commands, deps, sizeCounts.commandCount, sizeCounts.childCount);
			rules[ruleCount] = rule;
			ruleCount++;
			printCont(rule);
		}
	}
	//should somehow work it out if a file is entirely blank??
	//realloc array?
	return rules; 
}

static char* parseTarg(FILE *fptr, char* ch){

	if(*ch == ':'){
		//TODO INCORPORATE LINE NUMBER IN ERROR
		fprintf(stderr, "Target started with :, exiting");
		exit(-1);
	}

	size_t counter = 0;
	char* str = createStr(MAX_FILE_SIZE);
	//TODO Exit if EOF encountered in such a potential line
	//TODO ACCOUNT FOR SPACES
	while(*ch != EOF && *ch != ':' && counter < MAX_FILE_SIZE){
		str[counter] = *ch;
		counter++;
		*ch = fgetc(fptr);
	}

	checkEOF(ch);
	if(counter >= MAX_FILE_SIZE){
	//TODO TOO BIG PRINT TO STDERR WITH LINE NUM
	//	free(str);
		fprintf(stderr, "Exceeded file length");
		exit(-1);
	}

	str[counter] = '\0';
	*ch = fgetc(fptr); // need to do for future parsing
	return str;
	
}

static struct Target** parseChildren(FILE *fptr, char* ch, struct Sizes* sizeCounts){
	struct Target** deps = (struct Target**) malloc(sizeof(struct Target) * MAX_CHILDREN_SIZE);
	
	size_t childCount = 0;
	size_t lineLenCount = 0; //length of the string
	while(*ch != '\n' && lineLenCount < MAX_LINE_SIZE && *ch != EOF){
		skipWhitespace(fptr, ch);	

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
			fprintf(stderr,"Too long file length");	
			exit(-1);	
		}
		checkEOF(ch); //unexpected EOF (no commands to parse next)

		/* Newline found but no commands parsed, 
		if(numCommands == 0 && *ch == '\n'){
			free(str);
			fprintf(stderr, "Too long file length");
		}
		*/

		// Add null char, no need to increment where we are in the line
		if(childCount < MAX_CHILDREN_SIZE){
			str[fileLenCount] = '\0';
			fileLenCount = 0;
			struct Target* depen = createChild(str);
			deps[childCount] = depen;
			childCount++;
		}



		else{
			//TODO STRING TOO LONG OR RAN OUT OF SPACE FOR CHILDREN
		//	free(str);
			fprintf(stderr, "Too many files included");
			exit(-1);
		}
	}
	checkEOF(ch); 
	*ch = fgetc(fptr); //finished with this, move on
	sizeCounts->childCount = childCount;
	return deps;
}

static char** parseCommands(FILE *fptr, char* ch, struct Sizes* sizeCounts){
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

		if(currLineLen >= MAX_LINE_SIZE){
		//	free(str);
			fprintf(stderr, "Exceeded line length, exiting");
			exit(-1);
		}

		/*conditional if numCommands is zero but encountered new line?
		if(numCommands == 0 && *ch == '\t'){
			free(str);
			fprintf(stderr, "No commands found for that recipe");
			exit(-1);
		}
		*/

		if( numCommands < MAX_CHILDREN_SIZE){
			str[currLineLen] = '\0';
			currLineLen = 0;
			commands[numCommands] = str;
			numCommands++;
			*ch = fgetc(fptr); 
		}

		else{
		//	free(str);
			fprintf(stderr, "Too many commands, exiting");
			exit(-1);
		}
	}
	//don't check EOF here, handled by caller
	sizeCounts->commandCount = numCommands;
	return commands;
}

static void skipNewline(FILE *fptr, char* ch){ 
	if(*ch == '\n' && *ch != EOF){
		while(( *ch = fgetc(fptr)) != EOF && *ch == '\n');			
	}

}

static void skipWhitespace(FILE *fptr, char* ch){
	if(*ch == ' ' && *ch != EOF){
		while(( *ch = fgetc(fptr)) != EOF && *ch == ' ');
	}

	checkEOF(ch); 
	// maybe we had a target followed by spaces into EOF eg 
	// targ:                              EOF

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
