/**
 * acdat.c - Double-Array Trie implement
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#include "acdat.h"

#include <alib/object/list.h>

/* Trie 内部接口，仅限 Double-Array Trie 使用 */
size_t trie_size(trie_t self);

size_t trie_next_state_by_binary(trie_t self, size_t iNode, unsigned char key);

// Double-Array Trie
// ========================================================

const size_t DAT_ROOT_IDX = 255;

static inline dat_node_t dat_access_node(dat_t self, size_t index) {
  return segarray_access(self->node_array, index);
}

static void dat_init_segment(segarray_t segarray, void* segment, size_t seg_size, size_t start_index, void* arg) {
  dat_t datrie = (dat_t)arg;

  /* 节点初始化 */
  memset(segment, 0, segarray->node_size * seg_size);

  for (size_t i = 0; i < seg_size; ++i) {
    dat_node_t node = segarray_access(segarray, start_index + i);
    node->dat_free_next = start_index + i + 1;
    node->dat_free_last = start_index + i - 1;
  }
  ((dat_node_t)segarray_access(segarray, start_index + seg_size - 1))->dat_free_next = 0;

  ((dat_node_t)segarray_access(segarray, start_index))->dat_free_last = datrie->_sentinel->dat_free_last;
  ((dat_node_t)segarray_access(segarray, datrie->_sentinel->dat_free_last))->dat_free_next = start_index;
  datrie->_sentinel->dat_free_last = start_index + seg_size - 1;
}

static dat_node_t dat_access_node_with_alloc(dat_t self, size_t index) {
  dat_node_t node = segarray_access_s(self->node_array, index);
  if (node == NULL) {
    size_t extend_size = (index + 1) - segarray_size(self->node_array);
    if (segarray_extend(self->node_array, extend_size) != extend_size) {
      fprintf(stderr, "dat: alloc nodepool failed.\nexit.\n");
      exit(-1);
    }
    node = segarray_access_s(self->node_array, index);
  }
  return node;
}

static void dat_construct_by_dfs(dat_t self, trie_t origin, trie_node_t pNode, size_t datindex) {
  unsigned char child[256];
  int len = 0;
  trie_node_t pChild;
  size_t pos;

  dat_node_t pDatNode = dat_access_node(self, datindex);
  pDatNode->value = pNode->value;

  if (pNode->trie_child == 0) {
    return;
  }

  pChild = trie_access_node(origin, pNode->trie_child);
  while (pChild != origin->root) {
    child[len++] = pChild->key;
    pChild = trie_access_node(origin, pChild->trie_brother);
  }

  pos = self->_sentinel->dat_free_next;
  while (1) {
    int i;
    size_t base;

    if (pos == 0) {
      /* 扩容 */
      pos = self->_sentinel->dat_free_last;
      if (segarray_extend(self->node_array, 256) != 256) {
        fprintf(stderr, "alloc datnodepool failed: region full.\nexit.\n");
        exit(-1);
      }
      pos = dat_access_node(self, pos)->dat_free_next;
    }

    /* 检查: pos容纳第一个子节点 */
    base = pos - child[0];
    for (i = 1; i < len; ++i) {
      if (dat_access_node_with_alloc(self, base + child[i])->check != 0) {
        break;
      }
    }

    /* base 分配成功 */
    if (i == len) {
      pDatNode->base = base;
      for (i = 0; i < len; ++i) {
        /* 分配子节点 */
        dat_node_t pDatChild = dat_access_node(self, base + child[i]);
        pDatChild->check = datindex;
        pDatChild->value = NULL;
        /* remove the node from free list */
        dat_access_node(self, pDatChild->dat_free_next)->dat_free_last = pDatChild->dat_free_last;
        dat_access_node(self, pDatChild->dat_free_last)->dat_free_next = pDatChild->dat_free_next;
      }
      break;
    }

    pos = dat_access_node(self, pos)->dat_free_next;
  }

  /* 构建子树 */
  pChild = trie_access_node(origin, pNode->trie_child);
  while (pChild != origin->root) {
    pChild->trie_datidx = pDatNode->base + pChild->key;
    dat_construct_by_dfs(self, origin, pChild, pChild->trie_datidx);
    pChild = trie_access_node(origin, pChild->trie_brother);
  }
}

static void dat_post_construct(dat_t self) {
  /* 添加占位内存，防止匹配时出现非法访问 */
  segarray_extend(self->node_array, 256);
}

dat_t dat_alloc() {
  dat_t datrie = (dat_t)amalloc(sizeof(dat_s));
  if (datrie == NULL) {
    return NULL;
  }

  /* 节点初始化 */
  dat_node_s dummy_node = {0};
  datrie->_sentinel = &dummy_node;
  datrie->node_array = segarray_construct(sizeof(dat_node_s), dat_init_segment, datrie);
  segarray_extend(datrie->node_array, DAT_ROOT_IDX + 2);
  datrie->_sentinel = dat_access_node(datrie, 0);
  datrie->root = dat_access_node(datrie, DAT_ROOT_IDX);

  dat_access_node(datrie, dummy_node.dat_free_last)->dat_free_next = 0;
  dat_access_node(datrie, DAT_ROOT_IDX + 1)->dat_free_last = 0;

  /*
   * because type of base is 'size_t', we set free list start from index 256.
   * this can avoid negative number.
   */
  datrie->_sentinel->dat_free_next = DAT_ROOT_IDX + 1;
  datrie->_sentinel->dat_free_last = dummy_node.dat_free_last;

  return datrie;
}

void dat_construct_automation(dat_t self, trie_t origin) {
  size_t index;
  trie_node_t pNode = origin->root;
  size_t iChild = pNode->trie_child;
  while (iChild != 0) {
    trie_node_t pChild = trie_access_node(origin, iChild);

    /* 设置 failed 域 */
    iChild = pChild->trie_brother;
    pChild->trie_failed = 0;
    dat_access_node(self, pChild->trie_datidx)->failed = DAT_ROOT_IDX;
  }

  for (index = 1; index < trie_size(origin); index++) {  // bfs
    pNode = trie_access_node(origin, index);
    iChild = pNode->trie_child;
    while (iChild != 0) {
      trie_node_t pChild = trie_access_node(origin, iChild);
      unsigned char key = pChild->key;

      size_t iFailed = pNode->trie_failed;
      size_t match = trie_next_state_by_binary(origin, iFailed, key);
      while (iFailed != 0 && match == 0) {
        iFailed = trie_access_node(origin, iFailed)->trie_failed;
        match = trie_next_state_by_binary(origin, iFailed, key);
      }
      iChild = pChild->trie_brother;
      pChild->trie_failed = match;

      /* 设置 DAT 的 failed 域 */
      dat_access_node(self, pChild->trie_datidx)->failed =
          match == 0 ? DAT_ROOT_IDX : trie_access_node(origin, match)->trie_datidx;
    }
  }
}

void dat_destruct(dat_t dat, dat_node_free_f node_free_func) {
  if (dat != NULL) {
    if (node_free_func != NULL) {
      for (size_t i = 0; i < segarray_size(dat->node_array); i++) {
        dat_node_t node = dat_access_node(dat, i);
        if (node->check != 0 && node->value != NULL) {
          node_free_func(dat, node->value);
        }
      }
    }
    segarray_destruct(dat->node_array);
    afree(dat);
  }
}

dat_t dat_construct_by_trie(trie_t origin, bool enable_automation) {
  dat_t dat = dat_alloc();
  if (dat == NULL) {
    return NULL;
  }

  dat_construct_by_dfs(dat, origin, origin->root, DAT_ROOT_IDX);
  dat_post_construct(dat);
  if (enable_automation) {
    dat_construct_automation(dat, origin); /* 建立 AC 自动机 */
  }

  return dat;
}

// dat Context
// ===================================================

dat_ctx_t dat_alloc_context(dat_t datrie) {
  dat_ctx_t ctx = amalloc(sizeof(dat_ctx_s));
  if (ctx == NULL) {
    return NULL;
  }

  ctx->trie = datrie;

  return ctx;
}

bool dat_free_context(dat_ctx_t context) {
  if (context != NULL) {
    afree(context);
  }
  return true;
}

void dat_reset_context(dat_ctx_t context, char content[], size_t len) {
  context->content = (strlen_s){.ptr = content, .len = len};

  context->_read = 0;
  context->_begin = 0;
  context->_cursor = DAT_ROOT_IDX;
  context->_matched = context->trie->root;
}

bool dat_match_end(dat_ctx_t ctx) {
  return ctx->_read >= ctx->content.len;
}

static inline size_t dat_forward(dat_node_t cur, dat_ctx_t ctx) {
  return cur->base + ((unsigned char*)ctx->content.ptr)[ctx->_read];
}

bool dat_next_on_node(dat_ctx_t ctx) {
  dat_node_t pCursor = dat_access_node(ctx->trie, ctx->_cursor);
  for (; ctx->_read < ctx->content.len; ctx->_read++) {
    size_t iNext = dat_forward(pCursor, ctx);
    dat_node_t pNext = dat_access_node(ctx->trie, iNext);
    if (pNext->check != ctx->_cursor) {
      break;
    }
    ctx->_cursor = iNext;
    pCursor = pNext;
    if (pNext->value != NULL) {
      ctx->_matched = pNext;
      ctx->_read++;
      return true;
    }
  }

  for (ctx->_begin++; ctx->_begin < ctx->content.len; ctx->_begin++) {
    ctx->_cursor = DAT_ROOT_IDX;
    pCursor = ctx->trie->root;
    for (ctx->_read = ctx->_begin; ctx->_read < ctx->content.len; ctx->_read++) {
      size_t iNext = dat_forward(pCursor, ctx);
      dat_node_t pNext = dat_access_node(ctx->trie, iNext);
      if (pNext->check != ctx->_cursor) {
        break;
      }
      ctx->_cursor = iNext;
      pCursor = pNext;
      if (pNext->value != NULL) {
        ctx->_matched = pNext;
        ctx->_read++;
        return true;
      }
    }
  }
  return false;
}

bool dat_ac_next_on_node(dat_ctx_t ctx) {
  /* 检查当前匹配点向树根的路径上是否还有匹配的词 */
  while (ctx->_matched != ctx->trie->root) {
    ctx->_matched = dat_access_node(ctx->trie, ctx->_matched->failed);
    if (ctx->_matched->value != NULL) {
      return true;
    }
  }

  /* 执行匹配 */
  dat_node_t pCursor = dat_access_node(ctx->trie, ctx->_cursor);
  for (; ctx->_read < ctx->content.len; ctx->_read++) {
    size_t iNext = dat_forward(pCursor, ctx);
    dat_node_t pNext = dat_access_node(ctx->trie, iNext);
    while (pCursor != ctx->trie->root && pNext->check != ctx->_cursor) {
      ctx->_cursor = pCursor->failed;
      pCursor = dat_access_node(ctx->trie, ctx->_cursor);
      iNext = dat_forward(pCursor, ctx);
      pNext = dat_access_node(ctx->trie, iNext);
    }
    if (pNext->check == ctx->_cursor) {
      ctx->_cursor = iNext;
      pCursor = pNext;
      while (pNext != ctx->trie->root) {
        if (pNext->value != NULL) {
          ctx->_matched = pNext;
          ctx->_read++;
          return true;
        }
        pNext = dat_access_node(ctx->trie, pNext->failed);
      }
    }
  }
  return false;
}

bool dat_prefix_next_on_node(dat_ctx_t ctx) {
  /* 执行匹配 */
  dat_node_t pCursor = dat_access_node(ctx->trie, ctx->_cursor);
  for (; ctx->_read < ctx->content.len; ctx->_read++) {
    size_t iNext = dat_forward(pCursor, ctx);
    dat_node_t pNext = dat_access_node(ctx->trie, iNext);
    if (pNext->check != ctx->_cursor) {
      return false;
    }
    ctx->_cursor = iNext;
    pCursor = pNext;
    if (pNext->value != NULL) {
      ctx->_matched = pNext;
      ctx->_read++;
      return true;
    }
  }

  return false;
}
