// AUTHORS:
// MYKOLA KUSYY 
// GARRETT MCLAUGHLIN
#include <string.h>
#include <stdlib.h>
#include "target.h"
#include "parser.h"
#include "forker.h"
#include "mem_manage.h"

// unstatic it, put it in header file, get rid of nextRuleIndex
static struct Target* getRuleGraph(struct Rules* rules,  struct Target* currTarg); // for internal use
static struct Target* assignRule(struct Target* rule, struct Target* child);
static void detectDuplicates(struct Rules* rules);
static void checkCycle(struct Rules* rules);
static void isCycle(struct Target* target);

struct Rules* createGraph(struct Rules* rules){
	detectDuplicates(rules);
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
			fprintf(stderr, "File:%s not found for recipe:%s, exiting\n",
				       	targ->children[i]->target, targ->target);
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
		if(!strncmp(rules->rules[i]->target, desiredRule, rules->rules[i]->targetLen)){
			return rules->rules[i];
		}
	}
	return NULL; //found nothing
}

static struct Target* getRuleGraph(struct Rules* rules,  struct Target* currTarg){
	for(size_t i = 0; i < rules->numRules; i++){
		if(!strncmp(rules->rules[i]->target, currTarg->target, rules->rules[i]->targetLen)){
			return rules->rules[i];
		}
	}
	return NULL; //found nothing
}

static struct Target* assignRule(struct Target* rule, struct Target* child){
	struct Target* tmp = child;
	child = rule;
	freeAndNULL((void**)(&tmp->target));
	freeAndNULL((void**)(&tmp));
	return child;
}

static void detectDuplicates(struct Rules* rules){
	for(size_t i = 0; i < rules->numRules; i++){
		for(size_t j = i + 1; j < rules->numRules; j++){
			if(rules->rules[i]->targetLen > rules->rules[j]->targetLen){
				if(!strncmp(rules->rules[i]->target, rules->rules[j]->target, rules->rules[i]->targetLen)){
					fprintf(stderr, "Detected duplicate targets: %s, exiting\n", rules->rules[i]->target );
					exit(-1);
				}
			}
			else{
				if(!strncmp(rules->rules[i]->target, rules->rules[j]->target, rules->rules[i]->targetLen)){
					fprintf(stderr, "Detected duplicate targets: %s, exiting\n", rules->rules[i]->target );
					exit(-1);
				}
			}
		}
	}
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
