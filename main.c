// AUTHORS:
// MYKOLA KUSYY 
// GARRETT MCLAUGHLIN
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

// Find a makefile
// If it exists, we will be able to safely open it and pass it to parser
static int findMake(){
	//TODO Should be easy to implement extra credit with an arg here
	DIR *dirp = opendir(".");
	struct dirent *entry;

	const char* lilMake = "makefile";
	const char* bigMake = "Makefile";
	//extra credit const char* targetFile


	while((entry = readdir(dirp)) != NULL){
		if(!strcmp(entry->d_name, lilMake)){
			closedir(dirp);
			return 0;
		}

		if(!strcmp(entry->d_name, bigMake)){
			closedir(dirp);
			return 0;
		}
	}

	closedir(dirp);
	return -1; //failed to find
}


int main(void){
	int found = findMake();
	printf("OI OI FOUND YOU: %d", found);
	return 0;
}


