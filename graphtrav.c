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
	int needBuild = 0; //will use later to keep track
	for(size_t i = 0; i < targ->numChildren; i++){
		if(targ->modTime <= targ->children[i]->modTime){
			if(targ->children[i]->isRule){
				build(targ->children[i]);
			}
			//it's not a rule and the file doesn't exist
			if(targ->children[i]->modTime == 0){
				fprintf(stderr, "File not found for recipe, exiting");
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
	}
}
