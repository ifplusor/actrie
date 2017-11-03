#include "acdat.h"
#include "obj/list.h"

/* Trie 内部接口，仅限 Double-Array Trie 使用 */
size_t trie_size(trie_t self);

trie_node_t trie_access_node_export(trie_t self, size_t index);

size_t trie_next_state_by_binary(trie_t self, size_t iNode,
                                 unsigned char key);


// Double-Array Trie
// ========================================================

const matcher_func_l dat_matcher_func = {
    .destruct = (matcher_destruct_func) dat_destruct,
    .alloc_context = (matcher_alloc_context_func) dat_alloc_context
};

const context_func_l dat_context_func = {
    .free_context = (matcher_free_context_func) dat_free_context,
    .reset_context = (matcher_reset_context_func) dat_reset_context,
    .next = (matcher_next_func) dat_next_on_index
};

const context_func_l acdat_context_func = {
    .free_context = (matcher_free_context_func) dat_free_context,
    .reset_context = (matcher_reset_context_func) dat_reset_context,
    .next = (matcher_next_func) dat_ac_next_on_index
};

bool dat_dict_add_index(match_dict_t dict, aobj conf, strlen_s keyword,
                        strlen_s extra, void *tag, mdi_prop_f prop) {
  matcher_config_t config = GET_AOBJECT(conf);
  aobj stub_conf = ((stub_config_t) config->buf)->stub;
  matcher_config_t stub_config = GET_AOBJECT(stub_conf);
  return stub_config->add_index(dict, stub_conf, keyword, extra, tag, prop);
}

aobj dat_matcher_conf(uint8_t id, matcher_type_e type, aobj stub) {
  aobj conf = matcher_conf(id, type, dat_dict_add_index, sizeof(stub_config_s));
  if (conf) {
    matcher_config_t config = GET_AOBJECT(conf);
    config->clean = stub_config_clean;
    stub_config_t stub_config = (stub_config_t) config->buf;
    stub_config->stub = stub;
  }
  return conf;
}

const size_t DAT_ROOT_IDX = 255;

static inline dat_node_t dat_access_node(datrie_t self, size_t index) {
  size_t region = index >> REGION_OFFSET;
  size_t position = index & POSITION_MASK;
  return &self->_nodepool[region][position];
}

static void dat_alloc_nodepool(datrie_t self, size_t region) {
  size_t offset;
  int i;

  dat_node_t pnode =
      (dat_node_t) amalloc(sizeof(dat_node_s) * POOL_POSITION_SIZE);
  if (pnode == NULL) {
    fprintf(stderr, "dat: alloc nodepool failed.\nexit.\n");
    exit(-1);
  }
  self->_nodepool[region] = pnode;
  /* 节点初始化 */
  memset(pnode, 0, sizeof(dat_node_s) * POOL_POSITION_SIZE);
  offset = region << REGION_OFFSET;
  for (i = 0; i < POOL_POSITION_SIZE; ++i) {
    pnode[i].dat_free_next = offset + i + 1;
    pnode[i].dat_free_last = offset + i - 1;
  }
  pnode[POOL_POSITION_SIZE - 1].dat_free_next = 0;

  pnode[0].dat_free_last = self->_sentinel->dat_free_last;
  dat_access_node(self, self->_sentinel->dat_free_last)->dat_free_next = offset;
  self->_sentinel->dat_free_last = offset + POOL_POSITION_SIZE - 1;
}

static dat_node_t dat_access_node_with_alloc(datrie_t self,
                                               size_t index) {
  size_t region = index >> REGION_OFFSET;
  size_t position = index & POSITION_MASK;
  if (region >= POOL_REGION_SIZE) return NULL;
  if (self->_nodepool[region] == NULL) dat_alloc_nodepool(self, region);
  return &self->_nodepool[region][position];
}

static void dat_construct_by_dfs(datrie_t self, trie_t origin,
                                 trie_node_t pNode, size_t datindex) {
  unsigned char child[256];
  int len = 0;
  trie_node_t pChild;
  size_t pos;

  dat_node_t pDatNode = dat_access_node(self, datindex);
  pDatNode->dat_idxlist = pNode->trie_idxlist;
  _retain(pDatNode->dat_idxlist);

  if (pNode->trie_child == 0) return;

  pChild = trie_access_node_export(origin, pNode->trie_child);
  while (pChild != origin->root) {
    child[len++] = pChild->key;
    pChild = trie_access_node_export(origin, pChild->trie_brother);
  }
  pos = self->_sentinel->dat_free_next;
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
      if (dat_access_node_with_alloc(self, base + child[i])->check != 0) break;
    }

    /* base 分配成功 */
    if (i == len) {
      pDatNode->base = base;
      for (i = 0; i < len; ++i) {
        /* 分配子节点 */
        dat_node_t pDatChild = dat_access_node(self, base + child[i]);
        pDatChild->check = datindex;
        /* remove the node from free list */
        dat_access_node(self, pDatChild->dat_free_next)->dat_free_last =
            pDatChild->dat_free_last;
        dat_access_node(self, pDatChild->dat_free_last)->dat_free_next =
            pDatChild->dat_free_next;
      }
      break;
    }

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

static void dat_post_construct(datrie_t self) {
  /* 添加占位内存，防止匹配时出现非法访问 */
  int region = 1;
  while (region < POOL_REGION_SIZE && self->_nodepool[region] != NULL)
    ++region;
  if (region >= POOL_REGION_SIZE) {
    fprintf(stderr, "alloc datnodepool failed: region full.\nexit.\n");
    exit(-1);
  } else {
    dat_node_t pnode = (dat_node_t) amalloc(sizeof(dat_node_s) * 256);
    if (pnode == NULL) {
      fprintf(stderr, "alloc datnodepool failed.\nexit.\n");
      exit(-1);
    }
    self->_nodepool[region] = pnode;
    /* 节点初始化 */
    memset(pnode, 0, sizeof(dat_node_s) * 256);
  }
}

datrie_t dat_alloc() {
  dat_node_t pnode;
  datrie_t p;
  size_t i;

  pnode = (dat_node_t) amalloc(sizeof(dat_node_s) * POOL_POSITION_SIZE);
  if (pnode == NULL) {
    fprintf(stderr, "dat: alloc nodepool failed.\nexit.\n");
    return NULL;
  }
  memset(pnode, 0, sizeof(dat_node_s) * POOL_POSITION_SIZE);

  p = (datrie_t) amalloc(sizeof(datrie_s));
  if (p == NULL) {
    afree(pnode);
    return NULL;
  }

  for (i = 0; i < POOL_REGION_SIZE; ++i)
    p->_nodepool[i] = NULL;

  p->_nodepool[0] = pnode;
  p->_sentinel = &p->_nodepool[0][0];
  p->root = &p->_nodepool[0][DAT_ROOT_IDX];

  /* 节点初始化 */
  for (i = 1; i < POOL_POSITION_SIZE; ++i) {
    pnode[i].dat_free_next = i + 1;
    pnode[i].dat_free_last = i - 1;
  }
  pnode[POOL_POSITION_SIZE - 1].dat_free_next = 0;
  pnode[DAT_ROOT_IDX + 1].dat_free_last = 0;

  /*
   * because type of base is 'size_t', we set free list start from index 256.
   * this can avoid negative number.
   */
  p->_sentinel->dat_free_next = DAT_ROOT_IDX + 1;
  p->_sentinel->dat_free_last = POOL_POSITION_SIZE - 1;
  p->_sentinel->check = 1;

  return p;
}

void dat_construct_automation(datrie_t self, trie_t origin) {
  size_t index;
  trie_node_t pNode = origin->root;
  size_t iChild = pNode->trie_child;
  while (iChild != 0) {
    trie_node_t pChild = trie_access_node_export(origin, iChild);

    /* 设置 failed 域 */
    iChild = pChild->trie_brother;
    pChild->trie_failed = 0;
    dat_access_node(self, pChild->trie_datidx)->dat_failed = DAT_ROOT_IDX;
  }

  for (index = 1; index < trie_size(origin); index++) { // bfs
    pNode = trie_access_node_export(origin, index);
    iChild = pNode->trie_child;
    while (iChild != 0) {
      trie_node_t pChild = trie_access_node_export(origin, iChild);
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
//  fprintf(stderr, "construct AC automation succeed!\n");
}

bool dat_destruct(datrie_t p) {
  if (p != NULL) {
    for (int i = 0; i < POOL_REGION_SIZE; ++i) {
      if (p->_nodepool[i] == NULL) break;
      if (p->_nodepool[i + 1] != NULL) {
        // release list
        for (size_t j = 0; j < POOL_POSITION_SIZE; j++) {
          if (p->_nodepool[i][j].check == 0) continue;
          _release(p->_nodepool[i][j].dat_idxlist);
        }
      }
      afree(p->_nodepool[i]);
    }
    dict_release(p->_dict);
    afree(p);
    return true;
  }

  return false;
}

datrie_t dat_construct_by_trie(trie_t origin, bool enable_automation) {
  datrie_t p = dat_alloc();
  if (p == NULL)
    return NULL;

  p->_dict = dict_retain(origin->_dict);

  dat_construct_by_dfs(p, origin, origin->root, DAT_ROOT_IDX);
  dat_post_construct(p);
  if (enable_automation)
    dat_construct_automation(p, origin);    /* 建立 AC 自动机 */

//  fprintf(stderr, "construct double-array trie succeed!\n");
  return p;
}

matcher_t dat_construct(match_dict_t dict, aobj conf) {
  matcher_config_t config = GET_AOBJECT(conf);
  trie_t prime_trie = NULL;
  datrie_t pdat = NULL;

  do {
    trie_config_s trie_config = {
        .filter = config->id,
        .enable_automation = false
    };
    prime_trie = trie_construct(dict, &trie_config);
    if (prime_trie == NULL) break;

    if (config->type == matcher_type_dat) {
      pdat = dat_construct_by_trie(prime_trie, false);
    } else {
      pdat = dat_construct_by_trie(prime_trie, true);
    }
    if (pdat == NULL) break;

    pdat->header._type = config->type;
    pdat->header._func = dat_matcher_func;
  } while (0);

  trie_destruct(prime_trie);

  return (matcher_t) pdat;
}


// dat Context
// ===================================================

dat_context_t dat_alloc_context(datrie_t matcher) {
  dat_context_t ctx = amalloc(sizeof(dat_context_s));
  if (ctx == NULL) {
    return NULL;
  }

  ctx->trie = matcher;

  ctx->header._type = matcher->header._type;
  ctx->header._func = acdat_context_func;

  if (ctx->header._type == matcher_type_seg_acdat) {
    ctx->header._func.next = (matcher_next_func) dat_seg_ac_next_on_index;
  } else if (ctx->header._type == matcher_type_prefix_acdat) {
    ctx->header._func.next = (matcher_next_func) dat_prefix_next_on_index;
  } else if (ctx->header._type == matcher_type_dat) {
    ctx->header._func.next = (matcher_next_func) dat_next_on_index;
  }

  return ctx;
}

bool dat_free_context(dat_context_t context) {
  if (context != NULL) {
    afree(context);
  }

  return true;
}

bool dat_reset_context(dat_context_t context, unsigned char content[],
                       size_t len) {
  context->header.content = content;
  context->header.len = len;

  context->header.out_eo = 0;
  context->header.out_matched_index = NULL;

  context->_i = 0;
  context->_iCursor = DAT_ROOT_IDX;
  context->_matched = context->trie->root;
  context->_list = NULL;

  return true;
}

bool dat_match_end(dat_context_t ctx) {
  return ctx->header.out_eo >= ctx->header.len;
}

bool dat_next_on_index(dat_context_t ctx) {
  if (ctx->_list != NULL) {
    ctx->_list = _(list, ctx->_list, cdr);
    if (ctx->_list != NULL) {
      ctx->header.out_matched_index = _(list, ctx->_list, car);
      return true;
    }
  }

  dat_node_t pCursor = dat_access_node(ctx->trie, ctx->_iCursor);
  for (; ctx->header.out_eo < ctx->header.len; ctx->header.out_eo++) {
    size_t iNext = pCursor->base + ctx->header.content[ctx->header.out_eo];
    dat_node_t pNext = dat_access_node(ctx->trie, iNext);
    if (pNext->check != ctx->_iCursor)
      break;
    ctx->_iCursor = iNext;
    pCursor = pNext;
    if (pNext->dat_idxlist != NULL) {
      ctx->_matched = pNext;
      ctx->_list = ctx->_matched->dat_idxlist;
      ctx->header.out_matched_index = _(list, ctx->_list, car);
      ctx->header.out_eo++;
      return true;
    }
  }

  for (ctx->_i++; ctx->_i < ctx->header.len; ctx->_i++) {
    ctx->_iCursor = DAT_ROOT_IDX;
    pCursor = ctx->trie->root;
    for (ctx->header.out_eo = ctx->_i;
         ctx->header.out_eo < ctx->header.len;
         ctx->header.out_eo++) {
      size_t iNext = pCursor->base + ctx->header.content[ctx->header.out_eo];
      dat_node_t pNext = dat_access_node(ctx->trie, iNext);
      if (pNext->check != ctx->_iCursor)
        break;
      ctx->_iCursor = iNext;
      pCursor = pNext;
      if (pNext->dat_idxlist != NULL) {
        ctx->_matched = pNext;
        ctx->_list = ctx->_matched->dat_idxlist;
        ctx->header.out_matched_index = _(list, ctx->_list, car);
        ctx->header.out_eo++;
        return true;
      }
    }
  }
  return false;
}

bool dat_ac_next_on_node(dat_context_t ctx) {
  /* 检查当前匹配点向树根的路径上是否还有匹配的词 */
  while (ctx->_matched != ctx->trie->root) {
    ctx->_matched = dat_access_node(ctx->trie, ctx->_matched->dat_failed);
    if (ctx->_matched->dat_idxlist != NULL) {
      ctx->_list = ctx->_matched->dat_idxlist;
      ctx->header.out_matched_index = _(list, ctx->_list, car);
      return true;
    }
  }

  /* 执行匹配 */
  dat_node_t pCursor = dat_access_node(ctx->trie, ctx->_iCursor);
  for (; ctx->header.out_eo < ctx->header.len; ctx->header.out_eo++) {
    size_t iNext = pCursor->base + ctx->header.content[ctx->header.out_eo];
    dat_node_t pNext = dat_access_node(ctx->trie, iNext);
    while (pCursor != ctx->trie->root && pNext->check != ctx->_iCursor) {
      ctx->_iCursor = pCursor->dat_failed;
      pCursor = dat_access_node(ctx->trie, ctx->_iCursor);
      iNext = pCursor->base + ctx->header.content[ctx->header.out_eo];
      pNext = dat_access_node(ctx->trie, iNext);
    }
    if (pNext->check == ctx->_iCursor) {
      ctx->_iCursor = iNext;
      pCursor = pNext;
      while (pNext != ctx->trie->root) {
        if (pNext->dat_idxlist != NULL) {
          ctx->_matched = pNext;
          ctx->_list = ctx->_matched->dat_idxlist;
          ctx->header.out_matched_index = _(list, ctx->_list, car);
          ctx->header.out_eo++;
          return true;
        }
        pNext = dat_access_node(ctx->trie, pNext->dat_failed);
      }
    }
  }
  return false;
}

bool dat_ac_next_on_index(dat_context_t ctx) {
  /* 检查 index 列表 */
  if (ctx->_list != NULL) {
    ctx->_list = _(list, ctx->_list, cdr);
    if (ctx->_list != NULL) {
      ctx->header.out_matched_index = _(list, ctx->_list, car);
      return true;
    }
  }

  /* 检查当前匹配点向树根的路径上是否还有匹配的词 */
  while (ctx->_matched != ctx->trie->root) {
    ctx->_matched = dat_access_node(ctx->trie, ctx->_matched->dat_failed);
    if (ctx->_matched->dat_idxlist != NULL) {
      ctx->_list = ctx->_matched->dat_idxlist;
      ctx->header.out_matched_index = _(list, ctx->_list, car);
      return true;
    }
  }

  /* 执行匹配 */
  dat_node_t pCursor = dat_access_node(ctx->trie, ctx->_iCursor);
  for (; ctx->header.out_eo < ctx->header.len; ctx->header.out_eo++) {
    size_t iNext = pCursor->base + ctx->header.content[ctx->header.out_eo];
    dat_node_t pNext = dat_access_node(ctx->trie, iNext);
    while (pCursor != ctx->trie->root && pNext->check != ctx->_iCursor) {
      ctx->_iCursor = pCursor->dat_failed;
      pCursor = dat_access_node(ctx->trie, ctx->_iCursor);
      iNext = pCursor->base + ctx->header.content[ctx->header.out_eo];
      pNext = dat_access_node(ctx->trie, iNext);
    }
    if (pNext->check == ctx->_iCursor) {
      ctx->_iCursor = iNext;
      pCursor = pNext;
      while (pNext != ctx->trie->root) {
        if (pNext->dat_idxlist != NULL) {
          ctx->_matched = pNext;
          ctx->_list = ctx->_matched->dat_idxlist;
          ctx->header.out_matched_index = _(list, ctx->_list, car);
          ctx->header.out_eo++;
          return true;
        }
        pNext = dat_access_node(ctx->trie, pNext->dat_failed);
      }
    }
  }
  return false;
}

bool dat_seg_ac_next_on_index(dat_context_t ctx) {
  /* 检查 index 列表 */
  if (ctx->_list != NULL) {
    ctx->_list = _(list, ctx->_list, cdr);
    if (ctx->_list != NULL) {
      ctx->header.out_matched_index = _(list, ctx->_list, car);
      return true;
    }
  }

  /* 检查当前匹配点向树根的路径上是否还有匹配的词 */
  while (ctx->_matched != ctx->trie->root) {
    ctx->_matched = dat_access_node(ctx->trie, ctx->_matched->dat_failed);
    if (ctx->_matched->dat_idxlist != NULL) {
      ctx->_list = ctx->_matched->dat_idxlist;
      ctx->header.out_matched_index = _(list, ctx->_list, car);
      return true;
    }
  }

  /* 执行匹配 */
  dat_node_t pCursor = dat_access_node(ctx->trie, ctx->_iCursor);
  for (; ctx->header.out_eo < ctx->header.len; ctx->header.out_eo++) {
    size_t iNext = pCursor->base + ctx->header.content[ctx->header.out_eo];
    dat_node_t pNext = dat_access_node(ctx->trie, iNext);
    if (pCursor != ctx->trie->root) { // matching
      while (pCursor != ctx->trie->root && pNext->check != ctx->_iCursor) {
        // forward failed, backtrace
        ctx->_iCursor = pCursor->dat_failed;
        if (ctx->_iCursor == DAT_ROOT_IDX)
          return false; // back to root
        pCursor = dat_access_node(ctx->trie, ctx->_iCursor);
        iNext = pCursor->base + ctx->header.content[ctx->header.out_eo];
        pNext = dat_access_node(ctx->trie, iNext);
      }
    }
    if (pNext->check == ctx->_iCursor) {
      ctx->_iCursor = iNext;
      pCursor = pNext;
      while (pNext != ctx->trie->root) {
        if (pNext->dat_idxlist != NULL) {
          ctx->_matched = pNext;
          ctx->_list = ctx->_matched->dat_idxlist;
          ctx->header.out_matched_index = _(list, ctx->_list, car);
          ctx->header.out_eo++;
          return true;
        }
        pNext = dat_access_node(ctx->trie, pNext->dat_failed);
      }
    }
  }
  return false;
}

bool dat_prefix_next_on_index(dat_context_t ctx) {
  /* 检查 index 列表 */
  if (ctx->_list != NULL) {
    ctx->_list = _(list, ctx->_list, cdr);
    if (ctx->_list != NULL) {
      ctx->header.out_matched_index = _(list, ctx->_list, car);
      return true;
    }
  }

  /* 执行匹配 */
  dat_node_t pCursor = dat_access_node(ctx->trie, ctx->_iCursor);
  for (; ctx->header.out_eo < ctx->header.len; ctx->header.out_eo++) {
    size_t iNext = pCursor->base + ctx->header.content[ctx->header.out_eo];
    dat_node_t pNext = dat_access_node(ctx->trie, iNext);
    if (pNext->check != ctx->_iCursor) return false;
    ctx->_iCursor = iNext;
    pCursor = pNext;
    if (pNext->dat_idxlist != NULL) {
      ctx->_matched = pNext;
      ctx->_list = ctx->_matched->dat_idxlist;
      ctx->header.out_matched_index = _(list, ctx->_list, car);
      ctx->header.out_eo++;
      return true;
    }
  }
  return false;
}
