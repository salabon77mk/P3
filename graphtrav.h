// AUTHORS
// MYKOLA KUSYY
// GARRETT MCLAUGHLIN
#ifndef GRAPHTRAV_H
#define GRAPHTRAV_H

struct Rules* createGraph(struct Rules* rules);
void build(struct Target* targ); //recursive builder
struct Target* getRule(struct Rules* rules, char* desiredRule);
#endif

