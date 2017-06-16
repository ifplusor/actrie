#ifndef _MATCH_ACDAT_H_
#define _MATCH_ACDAT_H_


#include "actrie.h"
#include "matcher.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct dat_node {
	size_t base;
	size_t check;
	union {
		size_t next;
		size_t failed;
	} dat_nf;
#define dat_free_next   dat_nf.next
#define dat_failed      dat_nf.failed
	union {
		size_t last;
		match_dict_index_ptr dictidx;  /* out 表 */
	} dat_ld;
#define dat_free_last   dat_ld.last
#define dat_dictidx     dat_ld.dictidx
	int depth;  /* 可以从 strlen(dictidx.keyword) 导出 */
#define dat_depth       depth
} dat_node, *dat_node_ptr;

typedef struct dat_trie {
	struct _matcher header;

	dat_node_ptr    _nodepool[REGION_SIZE];
	dat_node_ptr    _lead;
	dat_node_ptr    root;
	match_dict_ptr  _dict;
} dat_trie, *dat_trie_ptr;

typedef struct dat_context {
	struct _context header;

	dat_trie_ptr    trie;

	dat_node_ptr    out_matched;
	dat_node_ptr    _pCursor;
	size_t          _iCursor;
	size_t          _i;

} dat_context, *dat_context_ptr;


dat_trie_ptr dat_construct_by_file(const char *path);
dat_trie_ptr dat_construct_by_string(const char *string);

void dat_destruct(dat_trie_ptr p);

dat_context_ptr dat_alloc_context(dat_trie_ptr matcher);
bool dat_free_context(dat_context_ptr context);
bool dat_reset_context(dat_context_ptr context, unsigned char content[], size_t len);

bool dat_next(dat_context_ptr context);

bool dat_ac_next_on_node(dat_context_ptr context);

bool dat_ac_next_on_index(dat_context_ptr context);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MATCH_ACDAT_H_ */
