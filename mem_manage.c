// AUTHORS:
// MYKOLA KUSYY 
// GARRETT MCLAUGHLIN
#include <stdlib.h>
#include <stdio.h>

char* createStr(size_t size){
	char* str = (char *) calloc(size,  sizeof(char));		
	if(str == NULL){
		fprintf(stderr, "malloc failed");
		exit(-1);
	}
	return str;
}

void* reallocWrapper(void* data, size_t newSize, size_t typeSize){
	void* newData = realloc(data, newSize * typeSize);	
	if( newData == NULL){
		free(newData);
		fprintf(stderr, "Failed to realloc in parser.c:doubleAllocatedMem");
		exit(-1);
	}
	data = newData;
	return data;
}

void* doubleAllocatedMem(void* data, size_t* currSize, size_t typeSize){
	void* newData =  realloc(data, ((*currSize) * 2) * typeSize);
	if( newData == NULL){
		free(newData);
		fprintf(stderr, "Failed to realloc in mem_manage.c:doubleAllocatedMem");
		exit(-1);
	}
	*currSize = (*currSize) * 2;
	data = newData;
	return data;
}

void* mallocWrapper(size_t dataType, size_t multiplier){
	void* data = malloc(dataType * multiplier);
	if(data == NULL){
		fprintf(stderr, "Malloc failed, exiting");
		exit(-1);
	}
	return data;
}

void freeAndNULL(void** ptr){
	free(*ptr);
	*ptr = NULL;
//	return ptr;
}
