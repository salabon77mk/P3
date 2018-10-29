// AUTHORS
// MYKOLA KUSYY
// GARRETT MCLAUGHLIN
#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>
#include <stdio.h>
#include "target.h"
struct Rules{
	struct Target** rules;
	size_t numRules;
};

struct Rules* parseRules(FILE *ptr);

#endif
