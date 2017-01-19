#ifndef _MATCH_ACDAT_H_
#define _MATCH_ACDAT_H_


#include "actrie.h"


typedef struct dat_node {
	size_t base, check;
	union {
		size_t next;
		size_t failed;
	} dat_nf;
#define dat_next   dat_nf.next
#define dat_failed dat_nf.failed
	union {
		size_t last;
		const char *extra;
	} dat_lf;
#define dat_last   dat_lf.last
#define dat_extra  dat_lf.extra
	const char *keyword;
#define dat_keyword keyword
	unsigned short depth;
#define dat_depth  depth
	unsigned char flag;
	unsigned char key;
#define dat_flag   flag
#define dat_key    key
} dat_node, *dat_node_ptr;

typedef struct dat_trie {
	dat_node_ptr _nodepool[REGION_SIZE];
	dat_node_ptr _lead;
	dat_node_ptr root;
	match_dict_ptr _dict;
} dat_trie, *dat_trie_ptr;

typedef struct dat_context {
	dat_trie_ptr trie;
	unsigned char *content;
	size_t len;

	dat_node_ptr out_matched;
	dat_node_ptr _pCursor;
	size_t _iCursor;
	size_t _i;
} dat_context, *dat_context_ptr;


dat_trie_ptr dat_construct(trie_ptr origin);

void dat_release(dat_trie_ptr p);

void dat_match(dat_trie_ptr self, unsigned char content[], size_t len);

void dat_construct_automation(dat_trie_ptr self, trie_ptr origin);

void dat_ac_match(dat_trie_ptr self, unsigned char content[], size_t len);

void dat_init_context(dat_context_ptr context, dat_trie_ptr trie,
					  unsigned char content[], size_t len);

bool dat_ac_next(dat_context_ptr context);


#endif // _MATCH_ACDAT_H_
