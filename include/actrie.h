#ifndef _MATCH_ACTRIE_H_
#define _MATCH_ACTRIE_H_


#include <stdio.h>
#include <memory.h>

#include "common.h"


typedef struct trienode *trienode_ptr;
typedef struct trie *trie_ptr;
typedef trienode_ptr (*nextnode_func)(trie_ptr,trienode_ptr, unsigned char);

typedef struct trienode {
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

typedef struct trie {
	trienode_ptr _nodepool[REGIONSIZE];
	size_t _autoindex;
	trienode_ptr trie_root;
} trie;

void initTrie(trie_ptr self);
void closeTrie(trie_ptr self);
bool constructTrie(trie_ptr self, FILE *fp);
void sortTrieForBinarySearch(trie_ptr self);
void rebuildTrieParent(trie_ptr self);
void constructTrieAutomation(trie_ptr self);
void matchActrie(trie_ptr self, unsigned char content[], size_t len);


#endif
