/**
 * acdat.c - Double-Array Trie implement
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#include "acdat.h"

#include <alib/collections/list/segarray.h>
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

  for (size_t i = start_index + 255; i < start_index + seg_size - 255; ++i) {
    dat_node_t node = segarray_access(segarray, i);
    node->dat_free_next = i + 1;
    node->dat_free_last = i - 1;
  }

  // segarray 是分段的，为索引转指针后不溢出，需要保留pad空间（掐头去尾）
  for (size_t i = 0; i < 255; ++i) {
    dat_node_t node = segarray_access(segarray, start_index + i);
    node->check.idx = 1;
    node = segarray_access(segarray, start_index + seg_size - 255 + i);
    node->check.idx = 1;
  }

  ((dat_node_t)segarray_access(segarray, start_index + 255))->dat_free_last = datrie->_sentinel->dat_free_last;
  ((dat_node_t)segarray_access(segarray, datrie->_sentinel->dat_free_last))->dat_free_next = start_index + 255;
  ((dat_node_t)segarray_access(segarray, start_index + seg_size - 256))->dat_free_next = 0;
  datrie->_sentinel->dat_free_last = start_index + seg_size - 256;
}

static dat_node_t dat_access_node_with_alloc(dat_t self, size_t index) {
  dat_node_t node = segarray_access_s(self->node_array, index);
  if (node == NULL) {
    size_t extend_size = (index + 1) - segarray_size(self->node_array);
    if (segarray_extend(self->node_array, extend_size) != extend_size) {
      fprintf(stderr, "dat: alloc nodepool failed.\nexit.\n");
      exit(-1);
    }
    node = segarray_access(self->node_array, index);
  }
  return node;
}

typedef struct dat_ctor_dfs_ctx {
  trie_node_t pNode, pChild;
} dat_ctor_dfs_ctx_s, *dat_ctor_dfs_ctx_t;

static void dat_construct_by_trie0(dat_t self, trie_t origin) {
  // set dat index of root
  origin->root->trie_datidx = DAT_ROOT_IDX;

  segarray_t stack = segarray_construct_with_type(dat_ctor_dfs_ctx_s);
  if (segarray_extend(stack, 2) != 2) {
    fprintf(stderr, "dat: alloc ctor_dfs_ctx failed.\nexit.\n");
    exit(-1);
  }
  dat_ctor_dfs_ctx_t ctx = (dat_ctor_dfs_ctx_t)segarray_access(stack, 1);
  ctx->pNode = origin->root;
  ctx->pChild = NULL;

  size_t stack_top = 1;
  while (stack_top > 0) {  // dfs
    ctx = (dat_ctor_dfs_ctx_t)segarray_access(stack, stack_top);

    // first visit
    if (ctx->pChild == NULL) {
      trie_node_t pNode = ctx->pNode;
      dat_node_t pDatNode = dat_access_node(self, pNode->trie_datidx);
      pDatNode->value.raw = pNode->value;

      // leaf node
      if (pNode->trie_child <= 0) {
        pDatNode->base.idx = 0;
        // pop stack
        stack_top--;
        continue;
      }

      uint8_t child[256];
      size_t len = 0;

      // fill key of children
      trie_node_t pChild = trie_access_node(origin, pNode->trie_child);
      while (pChild != origin->root) {
        child[len++] = pChild->key;
        pChild = trie_access_node(origin, pChild->trie_brother);
      }

      size_t pos = self->_sentinel->dat_free_next;
      while (1) {
        size_t base, i;

        /* 扩容 */
        if (pos == 0) {
          pos = self->_sentinel->dat_free_last;
          if (segarray_extend(self->node_array, 256) != 256) {
            fprintf(stderr, "alloc datnodepool failed: region full.\nexit.\n");
            exit(-1);
          }
          pos = dat_access_node(self, pos)->dat_free_next;
        }

        /* 检查: pos容纳第一个子节点 */
        base = pos - child[0];
        for (i = 0; i < len; ++i) {
          if (dat_access_node_with_alloc(self, base + child[i])->check.idx != 0) {
            break;
          }
        }

        /* base 分配成功 */
        if (i >= len) {
          pDatNode->base.idx = base;
          for (i = 0, pChild = trie_access_node(origin, pNode->trie_child); i < len;
               ++i, pChild = trie_access_node(origin, pChild->trie_brother)) {
            pChild->trie_datidx = base + child[i];
            /* 分配子节点 */
            dat_node_t pDatChild = dat_access_node(self, pChild->trie_datidx);
            /* remove the node from free list */
            dat_access_node(self, pDatChild->dat_free_next)->dat_free_last = pDatChild->dat_free_last;
            dat_access_node(self, pDatChild->dat_free_last)->dat_free_next = pDatChild->dat_free_next;
            // set fields
            pDatChild->check.idx = pNode->trie_datidx;
            pDatChild->value.raw = NULL;
          }
          break;
        }

        pos = dat_access_node(self, pos)->dat_free_next;
      }

      /* 构建子树 */
      ctx->pChild = trie_access_node(origin, pNode->trie_child);
      if (ctx->pChild != origin->root) {
        // push stack
        stack_top++;
        if (segarray_size(stack) <= stack_top && segarray_extend(stack, 1) != 1) {
          fprintf(stderr, "dat: alloc ctor_dfs_ctx failed.\nexit.\n");
          exit(-1);
        }
        dat_ctor_dfs_ctx_t ctx2 = (dat_ctor_dfs_ctx_t)segarray_access(stack, stack_top);
        ctx2->pNode = ctx->pChild;
        ctx2->pChild = NULL;
        continue;
      }
    } else {
      /* 构建子树 */
      ctx->pChild = trie_access_node(origin, ctx->pChild->trie_brother);
      if (ctx->pChild != origin->root) {
        // push stack
        stack_top++;
        dat_ctor_dfs_ctx_t ctx2 = (dat_ctor_dfs_ctx_t)segarray_access(stack, stack_top);
        ctx2->pNode = ctx->pChild;
        ctx2->pChild = NULL;
        continue;
      }
    }

    // pop stack
    stack_top--;
  }

  segarray_destruct(stack);
}

static void dat_post_construct(dat_t self, trie_t origin) {
  /* 添加占位内存，防止匹配时出现非法访问 */
  // segarray_extend(self->node_array, 256);

  // convert index to pointer
  size_t len = trie_size(origin);
  for (size_t index = 0; index < len; index++) {
    trie_node_t pNode = trie_access_node(origin, index);
    dat_node_t pDatNode = dat_access_node(self, pNode->trie_datidx);
    pDatNode->check.ptr = dat_access_node(self, pDatNode->check.idx);
    pDatNode->base.ptr = dat_access_node(self, pDatNode->base.idx);
    pDatNode->failed.ptr = dat_access_node(self, pDatNode->failed.idx);
  }

  if (self->enable_automation) {
    // 回溯优化
    self->value_array = segarray_construct(sizeof(dat_value_s), NULL, NULL);
    for (size_t index = 0; index < len; index++) {  // bfs
      trie_node_t pNode = trie_access_node(origin, index);
      dat_node_t pDatNode = dat_access_node(self, pNode->trie_datidx);
      if (pDatNode->value.raw != NULL) {
        if (segarray_extend(self->value_array, 1) != 1) {
          fprintf(stderr, "dat: alloc dat_value_s failed.\nexit.\n");
          exit(-1);
        }
        dat_value_t value = (dat_value_t)segarray_access(self->value_array, segarray_size(self->value_array) - 1);
        value->value = pDatNode->value.raw;
        value->next = pDatNode->failed.ptr->value.linked;
        pDatNode->value.linked = value;
      } else {
        pDatNode->value.linked = pDatNode->failed.ptr->value.linked;
      }
    }
  }
}

dat_t dat_alloc() {
  dat_t datrie = (dat_t)amalloc(sizeof(dat_s));
  if (datrie == NULL) {
    return NULL;
  }

  /* 节点初始化 */
  datrie->enable_automation = false;
  datrie->value_array = NULL;

  dat_node_s dummy_node = {0};
  datrie->_sentinel = &dummy_node;
  datrie->node_array = segarray_construct(sizeof(dat_node_s), dat_init_segment, datrie);
  segarray_extend(datrie->node_array, DAT_ROOT_IDX + 2);
  datrie->_sentinel = dat_access_node(datrie, 0);

  // set root node
  datrie->root = dat_access_node(datrie, DAT_ROOT_IDX);
  datrie->root->check.idx = DAT_ROOT_IDX;
  datrie->root->value.raw = NULL;

  // init free list
  dat_access_node(datrie, dummy_node.dat_free_last)->dat_free_next = 0;
  dat_access_node(datrie, DAT_ROOT_IDX + 1)->dat_free_last = 0;
  datrie->_sentinel->dat_free_next = DAT_ROOT_IDX + 1;
  datrie->_sentinel->dat_free_last = dummy_node.dat_free_last;

  return datrie;
}

void dat_build_automation(dat_t self, trie_t origin) {
  size_t index;
  trie_node_t pNode = origin->root;
  size_t iChild = pNode->trie_child;
  while (iChild != 0) {
    trie_node_t pChild = trie_access_node(origin, iChild);

    // must before set trie_failed
    iChild = pChild->trie_brother;

    /* 设置 failed 域 */
    pChild->trie_failed = 0;
    dat_access_node(self, pChild->trie_datidx)->failed.idx = DAT_ROOT_IDX;
  }

  size_t len = trie_size(origin);
  for (index = 1; index < len; index++) {  // bfs
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

      // must before set trie_failed
      iChild = pChild->trie_brother;

      /* 设置 failed 域 */
      pChild->trie_failed = match;
      dat_access_node(self, pChild->trie_datidx)->failed.idx =
          match == 0 ? DAT_ROOT_IDX : trie_access_node(origin, match)->trie_datidx;
    }
  }
}

void dat_destruct(dat_t dat, dat_node_free_f node_free_func) {
  if (dat != NULL) {
    if (node_free_func != NULL) {
      if (dat->enable_automation) {
        for (size_t i = 0; i < segarray_size(dat->value_array); i++) {
          dat_value_t value = (dat_value_t)segarray_access(dat->value_array, i);
          node_free_func(dat, value->value);
        }
      } else {
        for (size_t i = 0; i < segarray_size(dat->node_array); i++) {
          dat_node_t node = dat_access_node(dat, i);
          if (node->check.ptr != NULL && node->value.raw != NULL) {
            node_free_func(dat, node->value.raw);
          }
        }
      }
    }
    segarray_destruct(dat->node_array);
    segarray_destruct(dat->value_array);
    afree(dat);
  }
}

dat_t dat_construct_by_trie(trie_t origin, bool enable_automation) {
  dat_t dat = dat_alloc();
  if (dat == NULL) {
    return NULL;
  }

  dat_construct_by_trie0(dat, origin);
  if (enable_automation) {
    dat->enable_automation = true;
    dat_build_automation(dat, origin); /* 建立 AC 自动机 */
  }
  dat_post_construct(dat, origin);

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
  context->_cursor = context->trie->root;
  if (context->trie->enable_automation) {
    context->_matched.value = NULL;
  } else {
    context->_matched.node = context->trie->root;
  }
}

bool dat_match_end(dat_ctx_t ctx) {
  return ctx->_read >= ctx->content.len;
}

static inline dat_node_t dat_forward(dat_node_t cur, dat_ctx_t ctx) {
  return cur->base.ptr + ((uint8_t*)ctx->content.ptr)[ctx->_read];
}

bool dat_next_on_node(dat_ctx_t ctx) {
  dat_node_t pCursor = ctx->_cursor;
  for (; ctx->_read < ctx->content.len; ctx->_read++) {
    dat_node_t pNext = dat_forward(pCursor, ctx);
    if (pNext->check.ptr != pCursor) {
      break;
    }
    pCursor = pNext;
    if (pNext->value.raw != NULL) {
      ctx->_cursor = pCursor;
      ctx->_matched.node = pNext;
      ctx->_read++;
      return true;
    }
  }

  for (ctx->_begin++; ctx->_begin < ctx->content.len; ctx->_begin++) {
    pCursor = ctx->trie->root;
    for (ctx->_read = ctx->_begin; ctx->_read < ctx->content.len; ctx->_read++) {
      dat_node_t pNext = dat_forward(pCursor, ctx);
      if (pNext->check.ptr != pCursor) {
        break;
      }
      pCursor = pNext;
      if (pNext->value.raw != NULL) {
        ctx->_cursor = pCursor;
        ctx->_matched.node = pNext;
        ctx->_read++;
        return true;
      }
    }
  }

  return false;
}

bool dat_prefix_next_on_node(dat_ctx_t ctx) {
  /* 执行匹配 */
  dat_node_t pCursor = ctx->_cursor;
  for (; ctx->_read < ctx->content.len; ctx->_read++) {
    dat_node_t pNext = dat_forward(pCursor, ctx);
    if (pNext->check.ptr != pCursor) {
      return false;
    }
    pCursor = pNext;
    if (pNext->value.raw != NULL) {
      ctx->_cursor = pCursor;
      ctx->_matched.node = pNext;
      ctx->_read++;
      return true;
    }
  }

  return false;
}

bool dat_ac_next_on_node(dat_ctx_t ctx) {
  /* 检查当前匹配点向树根的路径上是否还有匹配的词 */
  while (ctx->_matched.value != NULL) {
    ctx->_matched.value = ctx->_matched.value->next;
    if (ctx->_matched.value != NULL) {
      return true;
    }
  }

  /* 执行匹配 */
  dat_node_t pCursor = ctx->_cursor;
  for (; ctx->_read < ctx->content.len; ctx->_read++) {
    dat_node_t pNext = dat_forward(pCursor, ctx);
    while (pCursor != ctx->trie->root && pNext->check.ptr != pCursor) {
      pCursor = pCursor->failed.ptr;
      pNext = dat_forward(pCursor, ctx);
    }
    if (pNext->check.ptr == pCursor) {
      pCursor = pNext;
      if (pNext->value.linked != NULL) {
        ctx->_cursor = pCursor;
        ctx->_matched.value = pNext->value.linked;
        ctx->_read++;
        return true;
      }
    }
    // else { pCursor == ctx->trie->root; }
  }

  ctx->_cursor = pCursor;
  // ctx->_matched = ctx->trie->root;
  return false;
}

bool dat_ac_prefix_next_on_node(dat_ctx_t ctx) {
  /* 执行匹配 */
  dat_node_t pCursor = ctx->_cursor;
  for (; ctx->_read < ctx->content.len; ctx->_read++) {
    dat_node_t pNext = dat_forward(pCursor, ctx);
    if (pNext->check.ptr != pCursor) {
      return false;
    }
    pCursor = pNext;
    if (pNext->value.linked != NULL) {
      ctx->_cursor = pCursor;
      ctx->_matched.value = pNext->value.linked;
      ctx->_read++;
      return true;
    }
  }

  return false;
}
