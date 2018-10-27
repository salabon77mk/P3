// AUTHORS:
// MYKOLA KUSYY 
// GARRETT MCLAUGHLIN
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "target.h"

static const unsigned int MAX_COMMANDS_SIZE = 1024;
static const unsigned int MAX_FILE_SIZE = 255; //Linux max file size

static char* getTarg(FILE *fptr, char* ch);


struct Target** parseRules(FILE *fptr){
	char ch;
	unsigned int lineNum = 0;
	
	while((ch = fgetc(fptr)) != EOF){
		//Skip blank lines
		if( ch == '\n'){
			while(( ch = fgetc(fptr)) != EOF && ch == '\n');			
		}
		
		struct Target* declaration = (struct Target*) malloc(sizeof(struct Target));
		//get target + command
		char* targ = getTarg(fptr, &ch);
		setTarget(targ, declaration);
		setModTimeTarget(declaration);
		
		return NULL; //temporary
	}

	return NULL; //temporary
}

static char* getTarg(FILE *fptr, char* ch){

	if(*ch == ':'){
		//TODO INCORPORATE LINE NUMBER IN ERROR
		fprintf(stderr, "Target started with :, exiting");
		exit(-1);
	}
	char* str = (char *) malloc(sizeof(char) * MAX_FILE_SIZE);
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
			//Do we call fgetc here?
			*ch = fgetc(fptr);
			return str;
		}
	}
	return NULL; //probably should be something else
}
