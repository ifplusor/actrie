/**
 * actrie.h
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#ifndef __ACTRIE_TRIE_H__
#define __ACTRIE_TRIE_H__

#include <alib/collections/list/segarray.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _trie_node_ { /* 十字链表实现字典树 */
  uint8_t key;
  int16_t len;  /* 子结点数量。一个结点只存储 1 byte 数据 */
  size_t child; /* 指向子结点 */
#define trie_child child
  union {
    /* failed 用于自动机，因为自动机构建时需要 bfs，而构建无队列 bfs 需要线性排序。
     * 因此，使用 failed 字段时已不需要 brother
     */
    size_t brother; /* 指向兄弟结点 */
    size_t failed;  /* 指向失败时跳跃结点 */
  } trie_bf_;
#define trie_brother trie_bf_.brother
#define trie_failed trie_bf_.failed
  void* value;
  union {
    size_t parent; /* 指向逻辑父结点，用于线性排序 */
    size_t datidx; /* 指向 dat 中对应结点 */
  } trie_pd_;
#define trie_parent trie_pd_.parent
#define trie_datidx trie_pd_.datidx
} trie_node_s, *trie_node_t;

typedef struct _trie_ {
  trie_node_t root;
  segarray_t node_array; /* 区位设计不需要大块连续内存，但不能用指针做遍历 */
} trie_s, *trie_t;

typedef void (*trie_node_free_f)(trie_t dat, void* node);

trie_t trie_alloc();
void trie_free(trie_t trie, trie_node_free_f node_free_func);

void* trie_add_keyword(trie_t self, const char* keyword, size_t len, void* value);
void* trie_search(trie_t self, const char* keyword, size_t len);
void trie_sort_to_line(trie_t self);
void trie_rebuild_parent_relation(trie_t self);
void trie_build_automation(trie_t self);

static inline trie_node_t trie_access_node(trie_t self, size_t index) {
#ifdef CHECK
  return segarray_access_s(self->node_array, index);
#else
  return segarray_access(self->node_array, index);
#endif  // CHECK
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  // __ACTRIE_TRIE_H__
