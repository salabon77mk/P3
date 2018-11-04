// AUTHORS:
// MYKOLA KUSYY 
// GARRETT MCLAUGHLIN
#ifndef MEM_MANAGE_H
#define MEM_MANAGE_H

char* createStr(size_t size);
void* doubleAllocatedMem(void* data, size_t* currSize, size_t typeSize); // make the size counter pbr
void* reallocWrapper(void* data, size_t newSize, size_t typeSize);
void* mallocWrapper(size_t dataType, size_t multiplier);
void* freeAndNULL(void* ptr);

#endif
