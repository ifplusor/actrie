#ifndef _MATCH_ACTRIE_H_
#define _MATCH_ACTRIE_H_


#include "common.h"


typedef struct trie_node *trie_node_ptr;
typedef struct trie *trie_ptr;

typedef trie_node_ptr (*next_node_func)(trie_ptr, trie_node_ptr, unsigned char);

typedef struct trie_node { // 十字链表实现字典树
	size_t child;     // 指向子结点
	size_t brother;   // 指向兄弟结点
	union {
		size_t parent;  // 指向逻辑父结点
		size_t datidx;  // 指向 dat 中对应结点
	} trie_pd;
#define trie_parent trie_pd.parent
#define trie_datidx trie_pd.datidx
	union { // 使用 prime trie 做 AC 自动机时，不能使用 extra 域
		const char *extra;
		size_t failed;  // 指向失败时跳跃结点
	} trie_ef;
#define trie_extra  trie_ef.extra
#define trie_failed trie_ef.failed
	union {
		const char *keyword;
		size_t placeholder;
	} trie_kp;
#define trie_keyword trie_kp.keyword
#define trie_p0      trie_kp.placeholder
	unsigned char len;  // 一个结点只存储 1 byte 数据
	unsigned char key;
	unsigned char flag;
} trie_node;

typedef struct trie {
	trie_node_ptr _nodepool[REGION_SIZE];
	size_t _autoindex;
	trie_node_ptr root;
} trie;

void trie_init(trie_ptr self);

void trie_close(trie_ptr self);

bool trie_construct(trie_ptr self, FILE *fp, match_dict_ptr dict);

void trie_sort_to_line(trie_ptr self);

void trie_rebuild_parent_relation(trie_ptr self);

void trie_construct_automation(trie_ptr self);

void trie_ac_match(trie_ptr self, unsigned char content[], size_t len);


#endif // _MATCH_ACTRIE_H_
