#ifndef _MATCH_ACDAT_H_
#define _MATCH_ACDAT_H_


#include "actrie.h"


typedef struct datnode {
	size_t base,check;
	union {
		size_t next;
		size_t failed;
	} nf;
	union {
		size_t last;
		struct {
			int depth;
			unsigned char flag;
			unsigned char key;
		} f;
	} lf;
} datnode, *datnode_ptr;

typedef struct datrie {
	datnode_ptr _datnodepool[REGIONSIZE], _datlead;
	datnode_ptr datroot;
} datrie, *datrie_ptr;

typedef struct datcontext {
	datrie_ptr trie;
	unsigned char *content;
	size_t len;

	datnode_ptr _pCursor;
	datnode_ptr _pMatched;
	size_t _iCursor;
	size_t _i;
} datcontext, *datcontext_ptr;

void initDat(datrie_ptr self);
void closeDat(datrie_ptr self);
void constructDat(datrie_ptr self, trie_ptr origin);
void matchDat(datrie_ptr self, unsigned char content[], size_t len);
void constructDatAutomation(datrie_ptr self, trie_ptr origin);
void matchAcdat(datrie_ptr self, unsigned char content[], size_t len);

void initAcdatContext(datcontext_ptr context, datrie_ptr trie, unsigned char content[], size_t len);
bool nextWithAcdat(datcontext_ptr context);
int getMatchedInAcdat(datcontext_ptr context, unsigned char buffer[], size_t size);


#endif
