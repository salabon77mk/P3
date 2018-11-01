// AUTHORS:
// MYKOLA KUSYY 
// GARRETT MCLAUGHLIN
#include <string.h>
#include <stdlib.h>
#include "target.h"
#include "parser.h"
#include "forker.h"
static struct Target* getRule(struct Rules* rules,  struct Target* currTarg, size_t nextRuleIndex);
static struct Target* assignRule(struct Target* rule, struct Target* child);

struct Rules* createGraph(struct Rules* rules){
	for(size_t i = 0; i < rules->numRules; i++){
		for(size_t j = 0; j < rules->rules[i]->numChildren; j++){
			struct Target* matchRule = getRule(rules, rules->rules[i]->children[j], (i + 1));
			if(matchRule != NULL){
				rules->rules[i]->children[j] = assignRule(matchRule, rules->rules[i]->children[j]);
			}
			//otherwise just move on
		}
	}
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

//Add an ASSIGNEDRULE var to struct for circular dependencies
static struct Target* getRule(struct Rules* rules,  struct Target* currTarg, size_t nextRuleIndex){
	for(size_t i = nextRuleIndex; i < rules->numRules; i++){
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

/* WITH CHILD CODE
struct Rules* createGraph(struct Rules* rules){
	for(size_t i = 0; i < rules->numRules; i++){
		for(size_t j = 0; j < rules->rules[i]->numChildren; j++){
			isRule(rules, rules->rules[i], rules->rules[i]->children[j], (i + 1)); //we're looking if a rule down the file exists
		}
	}
	return rules;
}

static int isRule(struct Rules* rules,  struct Target* currTarg, struct Target* currChild, size_t nextRuleIndex){
	for(size_t i = nextRuleIndex; i < rules->numRules; i++){
		if(!strncmp(rules->rules[i]->target, currChild->target, currChild->targetLen)){
			assignRule(rules->rules[i], currTarg, currCh);
			return 1;
		}
	}
	return 0; //false value in C
}

static void assignRule(struct Target* rule, struct Target* parent, size_t indexOfChild){
	struct Target* tmp = parent->children[indexOfChild];
	parent->children[indexOfChild] = child;
	free(tmp);
	tmp = NULL;
}
*/
