// AUTHORS:
// MYKOLA KUSYY 
// GARRETT MCLAUGHLIN
#include <string.h>
#include <stdlib.h>
#include "target.h"
#include "parser.h"
#include "forker.h"

// unstatic it, put it in header file, get rid of nextRuleIndex
static struct Target* getRuleGraph(struct Rules* rules,  struct Target* currTarg); // for internal use
static struct Target* assignRule(struct Target* rule, struct Target* child);
static void checkCycle(struct Rules* rules);
static void isCycle(struct Target* target);

struct Rules* createGraph(struct Rules* rules){
	for(size_t i = 0; i < rules->numRules; i++){
		for(size_t j = 0; j < rules->rules[i]->numChildren; j++){
			struct Target* matchRule = getRuleGraph(rules, rules->rules[i]->children[j]);
			if(matchRule != NULL){
				rules->rules[i]->children[j] = assignRule(matchRule, rules->rules[i]->children[j]);
			}
			//otherwise just move on
		}
	}
	checkCycle(rules);
	return rules;
}


void build(struct Target* targ){
	int needBuild = 1; //will use later to keep track
	for(size_t i = 0; i < targ->numChildren; i++){
		if(targ->children[i]->isRule){
			if(targ->children[i]->modTime == 0){
				build(targ->children[i]);
				needBuild = 1;
			}
			else if(targ->modTime < targ->children[i]->modTime){
				build(targ->children[i]);
				needBuild = 1;
			}
			else{
				needBuild = 0;
			}
			//otherwise it's up to date, no need to build
		}
		else if(targ->children[i]->modTime != 0){ // the file exists
			if(targ->modTime < targ->children[i]->modTime){
				needBuild = 1;
			}
			else{
				needBuild = 0;
			}
		}
		else{
			fprintf(stderr, "File:%s not found for recipe:%s, exiting\n", targ->children[i]->target, targ->target);
			exit(-1);
		}
	}

	if(needBuild){
		for(size_t i = 0; i < targ->numCommands; i++){
			runCommands(targ->commands[i], targ->target, targ->numArgs[i]);	
		}
	}
	else{
		fprintf(stderr, "Recipe for %s is up to date\n", targ->target);
	}	
}
struct Target* getRule(struct Rules* rules, char* desiredRule){
	for(size_t i = 0; i < rules->numRules; i++){
		if(!strncmp(rules->rules[i]->target, desiredRule, strlen(desiredRule))){
			return rules->rules[i];
		}
	}
	return NULL; //found nothing
}

//Add an ASSIGNEDRULE var to struct for circular dependencies
//
// UNSTATIC IT, INCLUDE IT IN MAIN, GET RID OF NEXTRULE INDEX
static struct Target* getRuleGraph(struct Rules* rules,  struct Target* currTarg){
	for(size_t i = 0; i < rules->numRules; i++){
		if(!strncmp(rules->rules[i]->target, currTarg->target, currTarg->targetLen)){
			return rules->rules[i];
		}
	}
	return NULL; //found nothing
}

static struct Target* assignRule(struct Target* rule, struct Target* child){
	struct Target* tmp = child;
	child = rule;
	free(tmp);
	tmp = NULL;
	return child;
}

static void checkCycle(struct Rules* rules){
	for(size_t i = 0; i < rules->numRules; i++){
		isCycle(rules->rules[i]);
	}

}
static void isCycle(struct Target* target){
	if(target->isRule){
		if(!target->visited){
			target->visited = 1;
			for(size_t i = 0; i < target->numChildren; i++){
				isCycle(target->children[i]);
			}
		}
		else{
		fprintf(stderr, "Detected cycle for rule: %s\n", target->target);		
		exit(-1);
		}
		target->visited = 0; //we can reset for recursive call
	}
}
/*
void build(struct Target* targ){
	int needBuild = 0; //will use later to keep track
	for(size_t i = 0; i < targ->numChildren; i++){
		if(targ->modTime >= targ->children[i]->modTime){
			if(targ->children[i]->isRule){
				build(targ->children[i]);
			}
			//it's not a rule and the file doesn't exist
			else if(targ->children[i]->modTime == 0x7fffffff){
				fprintf(stderr, "File:%s not found for recipe:%s, exiting\n", targ->children[i]->target, targ->target);
				exit(-1);
			}
			needBuild = 1;
		}
	}

	if(needBuild){
		for(size_t i = 0; i < targ->numCommands; i++){
			runCommands(targ->commands[i], targ->target);	
		}
	}
	else{
		fprintf(stderr, "Recipe for %s is up to date\n", targ->target);
	}	
}
		if(targ->modTime < targ->children[i]->modTime){
			if(targ->children[i]->isRule){
				build(targ->children[i]);
			}
			//it's not a rule and the file doesn't exist
			else if(targ->children[i]->modTime == 0){
				fprintf(stderr, "File:%s not found for recipe:%s, exiting\n", targ->children[i]->target, targ->target);
				exit(-1);
			}
			needBuild = 1;
		}
*/
