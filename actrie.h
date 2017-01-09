#ifndef _MATCH_ACTRIE_H_
#define _MATCH_ACTRIE_H_


#include <stdio.h>

#include "common.h"


typedef struct node *trienode_ptr;
typedef trienode_ptr (*nextnode_func)(trienode_ptr, unsigned char);

typedef struct node {
	size_t child,brother,failed;
	union {
		size_t parent;
		size_t datidx;
	} pd;
#ifdef ADVSCH
	nextnode_func next;
#endif
	unsigned short len;
	unsigned char flag;
	unsigned char key;
#if LOWERUPPER
	unsigned char lower;
	unsigned char upper;
#endif
} trienode;


void initTrie();
void closeTrie();
void constructTrie(FILE *fp);
void sortTrieForBinarySearch();
void rebuildTrieParent();
void constructTrieAutomation();
void matchActrie(unsigned char *content);


#endif
