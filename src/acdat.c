#include "acdat.h"

/* Trie 内部接口，仅限 Double-Array Trie 使用 */
size_t trie_size(trie_ptr self);

trie_node_ptr trie_access_node_export(trie_ptr self, size_t index);

size_t trie_next_state_by_binary(trie_ptr self, size_t iNode,
                                 unsigned char key);


// Double-Array Trie
// ========================================================

const matcher_func dat_matcher_func = {
    .destruct = (matcher_destruct_func) dat_destruct,
    .alloc_context = (matcher_alloc_context_func) dat_alloc_context
};

const context_func dat_context_func = {
    .free_context = (matcher_free_context_func) dat_free_context,
    .reset_context = (matcher_reset_context_func) dat_reset_context,
    .next = (matcher_next_func) dat_next_on_index
};

const context_func acdat_context_func = {
    .free_context = (matcher_free_context_func) dat_free_context,
    .reset_context = (matcher_reset_context_func) dat_reset_context,
    .next = (matcher_next_func) dat_ac_next_on_index
};

const size_t DAT_ROOT_IDX = 255;

static inline dat_node_ptr dat_access_node(dat_trie_ptr self, size_t index) {
  size_t region = index >> REGION_OFFSET;
  size_t position = index & POSITION_MASK;
  return &self->_nodepool[region][position];
}

static void dat_alloc_nodepool(dat_trie_ptr self, size_t region) {
  size_t offset;
  int i;

  dat_node_ptr pnode =
      (dat_node_ptr) malloc(sizeof(dat_node) * POOL_POSITION_SIZE);
  if (pnode == NULL) {
    fprintf(stderr, "dat: alloc nodepool failed.\nexit.\n");
    exit(-1);
  }
  self->_nodepool[region] = pnode;
  /* 节点初始化 */
  memset(pnode, 0, sizeof(dat_node) * POOL_POSITION_SIZE);
  offset = region << REGION_OFFSET;
  for (i = 0; i < POOL_POSITION_SIZE; ++i) {
    pnode[i].dat_free_next = offset + i + 1;
    pnode[i].dat_free_last = offset + i - 1;
  }
  pnode[POOL_POSITION_SIZE - 1].dat_free_next = 0;

  pnode[0].dat_free_last = self->_lead->dat_free_last;
  dat_access_node(self, self->_lead->dat_free_last)->dat_free_next = offset;
  self->_lead->dat_free_last = offset + POOL_POSITION_SIZE - 1;
}

static dat_node_ptr dat_access_node_with_alloc(dat_trie_ptr self,
                                               size_t index) {
  size_t region = index >> REGION_OFFSET;
  size_t position = index & POSITION_MASK;
  if (region >= POOL_REGION_SIZE) return NULL;
  if (self->_nodepool[region] == NULL) dat_alloc_nodepool(self, region);
  return &self->_nodepool[region][position];
}

//static size_t count = 0;

static void dat_construct_by_dfs(dat_trie_ptr self, trie_ptr origin,
                                 trie_node_ptr pNode, size_t datindex) {
  unsigned char child[256];
  int len = 0;
  trie_node_ptr pChild;
  size_t pos;

  dat_node_ptr pDatNode = dat_access_node(self, datindex);
  pDatNode->dat_dictidx = pNode->trie_dictidx;

  /*if (pDatNode->dat_flag & 2) {
      count++;
  }*/

  if (pNode->trie_child == 0) return;

  pChild = trie_access_node_export(origin, pNode->trie_child);
  while (pChild != origin->root) {
    child[len++] = pChild->key;
    pChild = trie_access_node_export(origin, pChild->trie_brother);
  }
  pos = self->_lead->dat_free_next;
  while (1) {
    int i;
    size_t base;

    if (pos == 0) {
      /* 扩容 */
      size_t region = 1;
      while (region < POOL_REGION_SIZE && self->_nodepool[region] != NULL)
        ++region;
      if (region == POOL_REGION_SIZE) {
        fprintf(stderr, "alloc datnodepool failed: region full.\nexit.\n");
        exit(-1);
      }
      dat_alloc_nodepool(self, region);
      pos = (size_t) region << REGION_OFFSET;
    }
    /* 检查: pos容纳第一个子节点 */
    base = pos - child[0];
    for (i = 1; i < len; ++i) {
      if (dat_access_node_with_alloc(self, base + child[i])->check != 0)
        goto checkfailed;
    }
    /* base 分配成功 */
    pDatNode->base = base;
    for (i = 0; i < len; ++i) {
      /* 分配子节点 */
      dat_node_ptr pDatChild = dat_access_node(self, base + child[i]);
      pDatChild->check = datindex;
      dat_access_node(self, pDatChild->dat_free_next)->dat_free_last =
          pDatChild->dat_free_last;
      dat_access_node(self, pDatChild->dat_free_last)->dat_free_next =
          pDatChild->dat_free_next;
      pDatChild->dat_depth = pDatNode->dat_depth + 1;
    }
    break;
checkfailed:
    pos = dat_access_node(self, pos)->dat_free_next;
  }

  /* 构建子树 */
  pChild = trie_access_node_export(origin, pNode->trie_child);
  while (pChild != origin->root) {
    pChild->trie_datidx = pDatNode->base + pChild->key;
    dat_construct_by_dfs(self, origin, pChild, pChild->trie_datidx);
    pChild = trie_access_node_export(origin, pChild->trie_brother);
  }
}

static void dat_post_construct(dat_trie_ptr self) {
  /* 添加占位内存，防止匹配时出现非法访问 */
  int region = 1;
  while (region < POOL_REGION_SIZE && self->_nodepool[region] != NULL)
    ++region;
  if (region >= POOL_REGION_SIZE) {
    fprintf(stderr, "alloc datnodepool failed: region full.\nexit.\n");
    exit(-1);
  } else {
    dat_node_ptr pnode = (dat_node_ptr) malloc(sizeof(dat_node) * 256);
    if (pnode == NULL) {
      fprintf(stderr, "alloc datnodepool failed.\nexit.\n");
      exit(-1);
    }
    self->_nodepool[region] = pnode;
    /* 节点初始化 */
    memset(pnode, 0, sizeof(dat_node) * 256);
  }
}

dat_trie_ptr dat_alloc() {
  dat_node_ptr pnode;
  dat_trie_ptr p;
  size_t i;

  pnode = (dat_node_ptr) malloc(sizeof(dat_node) * POOL_POSITION_SIZE);
  if (pnode == NULL) {
    fprintf(stderr, "dat: alloc nodepool failed.\nexit.\n");
    return NULL;
  }
  memset(pnode, 0, sizeof(dat_node) * POOL_POSITION_SIZE);

  p = (dat_trie_ptr) malloc(sizeof(dat_trie));
  if (p == NULL) {
    free(pnode);
    return NULL;
  }

  for (i = 0; i < POOL_REGION_SIZE; ++i)
    p->_nodepool[i] = NULL;

  p->_nodepool[0] = pnode;
  p->_lead = &p->_nodepool[0][0];
  p->root = &p->_nodepool[0][DAT_ROOT_IDX];

  /* 节点初始化 */
  for (i = 1; i < POOL_POSITION_SIZE; ++i) {
    pnode[i].dat_free_next = i + 1;
    pnode[i].dat_free_last = i - 1;
  }
  pnode[POOL_POSITION_SIZE - 1].dat_free_next = 0;
  pnode[DAT_ROOT_IDX + 1].dat_free_last = 0;

  p->_lead->dat_free_next = DAT_ROOT_IDX + 1;
  p->_lead->dat_free_last = POOL_POSITION_SIZE - 1;
  p->_lead->check = 1;

  p->root->dat_depth = 0;

  return p;
}

void dat_construct_automation(dat_trie_ptr self, trie_ptr origin) {
  size_t index;
  trie_node_ptr pNode = origin->root;
  size_t iChild = pNode->trie_child;
  while (iChild != 0) {
    trie_node_ptr pChild = trie_access_node_export(origin, iChild);

    /* 设置 failed 域 */
    iChild = pChild->trie_brother;
    pChild->trie_failed = 0;
    dat_access_node(self, pChild->trie_datidx)->dat_failed = DAT_ROOT_IDX;
  }

  for (index = 1; index < trie_size(origin); index++) { // bfs
    pNode = trie_access_node_export(origin, index);
    iChild = pNode->trie_child;
    while (iChild != 0) {
      trie_node_ptr pChild = trie_access_node_export(origin, iChild);
      unsigned char key = pChild->key;

      size_t iFailed = pNode->trie_failed;
      size_t match = trie_next_state_by_binary(origin, iFailed, key);
      while (iFailed != 0 && match == 0) {
        iFailed = trie_access_node_export(origin, iFailed)->trie_failed;
        match = trie_next_state_by_binary(origin, iFailed, key);
      }
      iChild = pChild->trie_brother;
      pChild->trie_failed = match;

      /* 设置 DAT 的 failed 域 */
      dat_access_node(self, pChild->trie_datidx)->dat_failed =
          match == 0 ? DAT_ROOT_IDX :
          trie_access_node_export(origin, match)->trie_datidx;
    }
  }
  fprintf(stderr, "construct AC automation succeed!\n");
}

bool dat_destruct(dat_trie_ptr p) {
  if (p != NULL) {
    int i;
    if (p->_dict != NULL) dict_release(p->_dict);
    for (i = 0; i < POOL_REGION_SIZE; ++i) {
      if (p->_nodepool[i] != NULL)
        free(p->_nodepool[i]);
    }
    free(p);
    return true;
  }

  return false;
}

dat_trie_ptr dat_construct(trie_ptr origin, bool enable_automation) {
  dat_trie_ptr p = dat_alloc();
  if (p == NULL)
    return NULL;

  p->_dict = dict_assign(origin->_dict);

  dat_construct_by_dfs(p, origin, origin->root, DAT_ROOT_IDX);
  dat_post_construct(p);
  if (enable_automation)
    dat_construct_automation(p, origin);    /* 建立 AC 自动机 */

  fprintf(stderr, "construct double-array trie succeed!\n");
  return p;
}

dat_trie_ptr dat_construct_by_file(const char *path, bool enable_automation) {
  trie_ptr prime_trie;
  dat_trie_ptr pdat;

  prime_trie = trie_construct_by_file(path, false);       /* 建立字典树 */
  if (prime_trie == NULL) return NULL;
  pdat = dat_construct(prime_trie,
                       enable_automation);    /* 建立 Double-Array Trie */
  trie_destruct(prime_trie);                              /* 释放字典树 */

  return pdat;
}

dat_trie_ptr dat_construct_by_string(const char *string,
                                     bool enable_automation) {
  trie_ptr prime_trie;
  dat_trie_ptr pdat;

  prime_trie = trie_construct_by_s(string, false);        /* 建立字典树 */
  if (prime_trie == NULL) return NULL;
  pdat = dat_construct(prime_trie,
                       enable_automation);    /* 建立 Double-Array Trie */
  trie_destruct(prime_trie);                              /* 释放字典树 */

  return pdat;
}


// dat Context
// ===================================================

dat_context_ptr dat_alloc_context(dat_trie_ptr matcher) {
  dat_context_ptr ctx = malloc(sizeof(dat_context));
  if (ctx == NULL) {
    return NULL;
  }

  ctx->trie = matcher;

  return ctx;
}

bool dat_free_context(dat_context_ptr context) {
  if (context != NULL) {
    free(context);
  }

  return true;
}

bool dat_reset_context(dat_context_ptr context, unsigned char content[],
                       size_t len) {
  context->header.content = content;
  context->header.len = len;

  context->header.out_e = 0;
  context->header.out_matched_index = NULL;

  context->_i = 0;
  context->_iCursor = DAT_ROOT_IDX;
  context->_pCursor = context->trie->root;
  context->out_matched = context->trie->root;

  return true;
}

bool dat_next_on_index(dat_context_ptr ctx) {
  if (ctx->header.out_matched_index != NULL) {
    ctx->header.out_matched_index = ctx->header.out_matched_index->_next;
    if (ctx->header.out_matched_index != NULL)
      return true;
  }

  for (; ctx->header.out_e < ctx->header.len; ctx->header.out_e++) {
    size_t iNext = ctx->_pCursor->base
        + ctx->header.content[ctx->header.out_e];
    dat_node_ptr pNext = dat_access_node(ctx->trie, iNext);
    if (pNext->check != ctx->_iCursor)
      break;
    ctx->_iCursor = iNext;
    ctx->_pCursor = pNext;
    if (pNext->dat_dictidx != NULL) {
      ctx->out_matched = pNext;
      ctx->header.out_matched_index = ctx->out_matched->dat_dictidx;
      ctx->header.out_e++;
      return true;
    }
  }

  for (ctx->_i++; ctx->_i < ctx->header.len; ctx->_i++) {
    ctx->_iCursor = DAT_ROOT_IDX;
    ctx->_pCursor = ctx->trie->root;
    for (ctx->header.out_e = ctx->_i;
         ctx->header.out_e < ctx->header.len;
         ctx->header.out_e++) {
      size_t iNext = ctx->_pCursor->base
          + ctx->header.content[ctx->header.out_e];
      dat_node_ptr pNext = dat_access_node(ctx->trie, iNext);
      if (pNext->check != ctx->_iCursor)
        break;
      ctx->_iCursor = iNext;
      ctx->_pCursor = pNext;
      if (pNext->dat_dictidx != NULL) {
        ctx->out_matched = pNext;
        ctx->header.out_matched_index = ctx->out_matched->dat_dictidx;
        ctx->header.out_e++;
        return true;
      }
    }
  }
  return false;
}

bool dat_ac_next_on_node(dat_context_ptr ctx) {
  /* 检查当前匹配点向树根的路径上是否还有匹配的词 */
  while (ctx->out_matched != ctx->trie->root) {
    ctx->out_matched =
        dat_access_node(ctx->trie, ctx->out_matched->dat_failed);
    if (ctx->out_matched->dat_dictidx != NULL) {
      ctx->header.out_matched_index = ctx->out_matched->dat_dictidx;
      return true;
    }
  }

  /* 执行匹配 */
  for (; ctx->header.out_e < ctx->header.len; ctx->header.out_e++) {
    size_t iNext = ctx->_pCursor->base
        + ctx->header.content[ctx->header.out_e];
    dat_node_ptr pNext = dat_access_node(ctx->trie, iNext);
    while (ctx->_pCursor != ctx->trie->root
        && pNext->check != ctx->_iCursor) {
      ctx->_iCursor = ctx->_pCursor->dat_failed;
      ctx->_pCursor = dat_access_node(ctx->trie, ctx->_iCursor);
      iNext = ctx->_pCursor->base
          + ctx->header.content[ctx->header.out_e];
      pNext = dat_access_node(ctx->trie, iNext);
    }
    if (pNext->check == ctx->_iCursor) {
      ctx->_iCursor = iNext;
      ctx->_pCursor = pNext;
      while (pNext != ctx->trie->root) {
        if (pNext->dat_dictidx != NULL) {
          ctx->out_matched = pNext;
          ctx->header.out_matched_index = ctx->out_matched->dat_dictidx;
          ctx->header.out_e++;
          return true;
        }
        pNext = dat_access_node(ctx->trie, pNext->dat_failed);
      }
    }
  }
  return false;
}

bool dat_ac_next_on_index(dat_context_ptr ctx) {
  /* 检查 index 列表 */
  if (ctx->header.out_matched_index != NULL) {
    ctx->header.out_matched_index = ctx->header.out_matched_index->_next;
    if (ctx->header.out_matched_index != NULL)
      return true;
  }

  /* 检查当前匹配点向树根的路径上是否还有匹配的词 */
  while (ctx->out_matched != ctx->trie->root) {
    ctx->out_matched =
        dat_access_node(ctx->trie, ctx->out_matched->dat_failed);
    if (ctx->out_matched->dat_dictidx != NULL) {
      ctx->header.out_matched_index = ctx->out_matched->dat_dictidx;
      return true;
    }
  }

  /* 执行匹配 */
  for (; ctx->header.out_e < ctx->header.len; ctx->header.out_e++) {
    size_t iNext = ctx->_pCursor->base
        + ctx->header.content[ctx->header.out_e];
    dat_node_ptr pNext = dat_access_node(ctx->trie, iNext);
    while (ctx->_pCursor != ctx->trie->root
        && pNext->check != ctx->_iCursor) {
      ctx->_iCursor = ctx->_pCursor->dat_failed;
      ctx->_pCursor = dat_access_node(ctx->trie, ctx->_iCursor);
      iNext = ctx->_pCursor->base
          + ctx->header.content[ctx->header.out_e];
      pNext = dat_access_node(ctx->trie, iNext);
    }
    if (pNext->check == ctx->_iCursor) {
      ctx->_iCursor = iNext;
      ctx->_pCursor = pNext;
      while (pNext != ctx->trie->root) {
        if (pNext->dat_dictidx != NULL) {
          ctx->out_matched = pNext;
          ctx->header.out_matched_index = ctx->out_matched->dat_dictidx;
          ctx->header.out_e++;
          return true;
        }
        pNext = dat_access_node(ctx->trie, pNext->dat_failed);
      }
    }
  }
  return false;
}

bool dat_prefix_next_on_index(dat_context_ptr ctx) {
  /* 检查 index 列表 */
  if (ctx->header.out_matched_index != NULL) {
    ctx->header.out_matched_index = ctx->header.out_matched_index->_next;
    if (ctx->header.out_matched_index != NULL)
      return true;
  }

  /* 执行匹配 */
  for (; ctx->header.out_e < ctx->header.len; ctx->header.out_e++) {
    size_t iNext = ctx->_pCursor->base
        + ctx->header.content[ctx->header.out_e];
    dat_node_ptr pNext = dat_access_node(ctx->trie, iNext);
    if (pNext->check != ctx->_iCursor) return false;
    ctx->_iCursor = iNext;
    ctx->_pCursor = pNext;
    if (pNext->dat_dictidx != NULL) {
      ctx->out_matched = pNext;
      ctx->header.out_matched_index = ctx->out_matched->dat_dictidx;
      ctx->header.out_e++;
      return true;
    }
  }
  return false;
}
