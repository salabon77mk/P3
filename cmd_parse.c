// AUTHORS:
// MYKOLA KUSYY 
// GARRETT MCLAUGHLIN
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include "cmd_parse.h"
static const size_t MAX_FILE_SIZE = 256; //Linux max file size, 255 reserved for NULL
static struct parsedCMD* create_parsedCMD();

struct parsedCMD* parseCMD(int argc, char** argv){
	struct parsedCMD* cmdFlags = create_parsedCMD();
	int c;
	int index;

	while((c = getopt(argc, argv, "f:")) != -1){
		switch(c){
			case 'f':
				if(strlen(optarg) < MAX_FILE_SIZE){
					cmdFlags->fFlag = 1;
					cmdFlags->specifiedMakefile = optarg;
					cmdFlags->makefileSize = strlen(optarg);
					break;
				}
				else{
					fprintf(stderr, "File length is too long, exiting");
					exit(-1);
				}
				break;
			case '?':
				if(optopt == 'f'){
					fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				}
				else if(isprint(optopt)){
					fprintf(stderr, "Unknown option -%c. \n", optopt);
				}
				else{
					fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
				}
				break;
			default:
				abort();
		}
	}
	// take the first arg as the specified rule
	for(index = optind; index < argc; index++){
			if(strlen(argv[index]) < MAX_FILE_SIZE){
				cmdFlags->specifiedRule = argv[index];
				break;
			}
			else{
				fprintf(stderr, "Argument: %s is too long, exiting", argv[0]);
				exit(-1);
			}
		//otherwise we're just going to the end
	}
	return cmdFlags;
}

static struct parsedCMD* create_parsedCMD(){
	struct parsedCMD* cmdFlags = (struct parsedCMD*) malloc(sizeof(struct parsedCMD));
	if(cmdFlags == NULL){
		fprintf(stderr, "Malloc failed in cmd_parse.c");
		exit(-1);
	}
		
	cmdFlags->specifiedRule = NULL;
	cmdFlags->specifiedMakefile = NULL;
	
	cmdFlags->makefileSize = 0;
	cmdFlags->fFlag = 0;

	return cmdFlags;
}
