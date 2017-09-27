#include "actrie.h"


// Prime Trie
// ========================================================

size_t trie_size(trie_ptr self) {
  return self->_autoindex;
}

static size_t trie_alloc_node(trie_ptr self) {
  size_t region = self->_autoindex >> REGION_OFFSET;
//	size_t position = self->_autoindex & POSITION_MASK;
#ifdef CHECK
  if (region >= POOL_REGION_SIZE)
      return (size_t)-1;
#endif // CHECK
  if (self->_nodepool[region] == NULL) {
    trie_node_ptr pnode =
        (trie_node_ptr) malloc(sizeof(trie_node) * POOL_POSITION_SIZE);
    if (pnode == NULL) return (size_t) -1;
    self->_nodepool[region] = pnode;
    memset(pnode, 0, sizeof(trie_node) * POOL_POSITION_SIZE);
  }
#ifdef CHECK
  if (self->_autoindex == (size_t)-1)
      return (size_t)-1;
#endif // CHECK
  return self->_autoindex++;
}

#if defined(_WIN32) && !defined(__cplusplus)
#define inline __inline
#endif

static inline trie_node_ptr trie_access_node(trie_ptr self, size_t index) {
  size_t region = index >> REGION_OFFSET;
  size_t position = index & POSITION_MASK;
#ifdef CHECK
  if (region >= POOL_REGION_SIZE || self->_nodepool[region] == NULL
          || index >= self->_autoindex) {
      return NULL;
  }
#endif // CHECK
  return &self->_nodepool[region][position];
}

trie_node_ptr trie_access_node_export(trie_ptr self, size_t index) {
  size_t region = index >> REGION_OFFSET;
  size_t position = index & POSITION_MASK;
#ifdef CHECK
  if (region >= POOL_REGION_SIZE || self->_nodepool[region] == NULL
          || index >= self->_autoindex) {
      return NULL;
  }
#endif // CHECK
  return &self->_nodepool[region][position];
}

bool trie_add_keyword(trie_ptr self, const unsigned char keyword[], size_t len,
                      match_dict_index_ptr index) {
  trie_node_ptr pNode = self->root;
  size_t iNode = 0; /* iParent保存pNode的index */
  size_t i = 0;

  for (i = 0; i < len; ++i) {
    /* 当创建时，使用插入排序的方法，以保证子节点链接关系有序 */
    size_t iChild = pNode->trie_child, iBrother = 0; // iBrother跟踪iChild
    trie_node_ptr pChild = NULL;
    while (iChild != 0) {
      /* 从所有孩子中查找 */
      pChild = trie_access_node(self, iChild);
      if (pChild->key >= keyword[i]) break;
      iBrother = iChild;
      iChild = pChild->trie_brother;
    }

    if (iChild != 0 && pChild->key == keyword[i]) {
      /* 找到 */
      iNode = iChild;
      pNode = pChild;
    } else {
      /* 没找到, 创建. */
      size_t idx = trie_alloc_node(self);
      trie_node_ptr pc = NULL;

      if (idx == -1) return false;

      pc = trie_access_node(self, idx);
      if (pc == NULL) return false;
      pc->key = keyword[i];

      if (pChild == NULL) {
        /* 没有子节点 */
        pNode->trie_child = idx;
        pc->trie_parent = iNode;
      } else {
        if (iBrother == 0) {
          /* 插入链表头 */
          pc->trie_parent = iNode;
          pc->trie_brother = pNode->trie_child;
          pNode->trie_child = idx;
          pChild->trie_parent = idx;
        } else if (pChild->key < keyword[i]) {
          /* 插入链表尾 */
          pc->trie_parent = iBrother;
          pChild->trie_brother = idx;
        } else {
          trie_node_ptr pBrother = trie_access_node(self, iBrother);
          pc->trie_parent = iBrother;
          pc->trie_brother = iChild;
          pBrother->trie_brother = idx;
          pChild->trie_parent = idx;
        }
      }

      pNode->len++;
      iNode = idx;
      pNode = pc;
    }
  }

  if (pNode->trie_dictidx != NULL)
    index->_next = pNode->trie_dictidx;
  pNode->trie_dictidx = index;

  return true;
}

size_t trie_next_state_by_binary(trie_ptr self,
                                 size_t iNode,
                                 unsigned char key) {
  trie_node_ptr pNode = trie_access_node(self, iNode);
  if (pNode->len >= 1) {
    size_t left = pNode->trie_child;
    size_t right = left + pNode->len - 1;
    if (key < trie_access_node(self, left)->key ||
        trie_access_node(self, right)->key < key)
      return 0;
    while (left <= right) {
      size_t middle = (left + right) >> 1;
      trie_node_ptr pMiddle = trie_access_node(self, middle);
      if (pMiddle->key == key) {
        return middle;
      } else if (pMiddle->key > key) {
        right = middle - 1;
      } else {
        left = middle + 1;
      }
    }
  }
  return 0;
}

trie_node_ptr trie_next_node_by_binary(trie_ptr self, trie_node_ptr pNode,
                                       unsigned char key) {
  size_t left, right;
  if (key < trie_access_node(self, pNode->trie_child)->key ||
      trie_access_node(self, pNode->trie_child + pNode->len - 1)->key < key)
    return self->root;

  left = pNode->trie_child;
  right = left + pNode->len - 1;
  while (left <= right) {
    size_t middle = (left + right) >> 1;
    trie_node_ptr pMiddle = trie_access_node(self, middle);
    if (pMiddle->key < key) {
      left = middle + 1;
    } else if (pMiddle->key > key) {
      right = middle - 1;
    } else {
      return pMiddle;
    }
  }

  return self->root;
}

void trie_swap_node_data(trie_node_ptr pa, trie_node_ptr pb) {
  pa->trie_child ^= pb->trie_child;
  pb->trie_child ^= pa->trie_child;
  pa->trie_child ^= pb->trie_child;

  pa->trie_brother ^= pb->trie_brother;
  pb->trie_brother ^= pa->trie_brother;
  pa->trie_brother ^= pb->trie_brother;

  pa->trie_parent ^= pb->trie_parent;
  pb->trie_parent ^= pa->trie_parent;
  pa->trie_parent ^= pb->trie_parent;

  // dict index
  pa->trie_p0 ^= pb->trie_p0;
  pb->trie_p0 ^= pa->trie_p0;
  pa->trie_p0 ^= pb->trie_p0;

  pa->len ^= pb->len;
  pb->len ^= pa->len;
  pa->len ^= pb->len;

  pa->key ^= pb->key;
  pb->key ^= pa->key;
  pa->key ^= pb->key;
}

// swap and return iChild's brother node
size_t trie_swap_node(trie_ptr self, size_t iChild, size_t iTarget) {
  trie_node_ptr pChild = trie_access_node(self, iChild);
  if (iChild != iTarget) {
    trie_node_ptr ptmp, pTarget = trie_access_node(self, iTarget);

    /* 常量 */
    const size_t ipc = pChild->trie_parent;
    const size_t icc = pChild->trie_child;
    const size_t ibc = pChild->trie_brother;
    const bool bcc = trie_access_node(self, ipc)->trie_child == iChild;
    const size_t ipt = pTarget->trie_parent;
    const size_t ict = pTarget->trie_child;
    const size_t ibt = pTarget->trie_brother;
    const bool bct = trie_access_node(self, ipt)->trie_child == iTarget;

    /* 交换节点内容 */
    trie_swap_node_data(pChild, pTarget);
    ptmp = pChild;
    pChild = pTarget;
    pTarget = ptmp;

    /* 考虑上下级节点交换
     * 调整target的上级，child的下级
     */
    if (ipt == iChild) {
      /* target是child的下级，child是target的上级 */
      pTarget->trie_parent = iTarget;
      if (bct) {
        pChild->trie_child = iChild;
        if (ibc != 0)
          trie_access_node(self, ibc)->trie_parent = iTarget;
      } else {
        pChild->trie_brother = iChild;
        if (icc != 0)
          trie_access_node(self, icc)->trie_parent = iTarget;
      }
    } else {
      if (bct)
        trie_access_node(self, ipt)->trie_child = iChild;
      else
        trie_access_node(self, ipt)->trie_brother = iChild;
      if (icc != 0) trie_access_node(self, icc)->trie_parent = iTarget;
      if (ibc != 0) trie_access_node(self, ibc)->trie_parent = iTarget;
    }

    /* 调整直接上级指针 */
    if (bcc)
      trie_access_node(self, ipc)->trie_child = iTarget;
    else
      trie_access_node(self, ipc)->trie_brother = iTarget;

    /* 调整下级指针 */
    if (ict != 0) trie_access_node(self, ict)->trie_parent = iChild;
    if (ibt != 0) trie_access_node(self, ibt)->trie_parent = iChild;
  }
  return pChild->trie_brother;
}

trie_ptr trie_alloc() {
  size_t root;
  int i;

  trie_ptr p = (trie_ptr) malloc(sizeof(trie));
  if (p == NULL)
    goto trie_alloc_failed;

  p->_dict = NULL;
  for (i = 0; i < POOL_REGION_SIZE; i++)
    p->_nodepool[i] = NULL;
  p->_autoindex = 0;

  root = trie_alloc_node(p);
  if (root == (size_t) -1)
    goto trie_alloc_failed;

  p->root = trie_access_node(p, root);
  if (p->root == NULL)
    goto trie_alloc_failed;

  return p;

trie_alloc_failed:
  trie_destruct(p);
  return NULL;
}

void trie_sort_to_line(trie_ptr self) {
  size_t i, iTarget = 1;
  for (i = 0; i < iTarget; i++) { /* 隐式bfs队列 */
    trie_node_ptr pNode = trie_access_node(self, i);
    /* 将pNode的子节点调整到iTarget位置（加入队列） */
    size_t iChild = pNode->trie_child;
    while (iChild != 0) {
      /* swap iChild与iTarget
      * 建树时的尾插法担保兄弟节点不会交换，且在重排后是稳定的
      */
      iChild = trie_swap_node(self, iChild, iTarget);
      iTarget++;
    }
  }
  fprintf(stderr, "sort succeed!\n");
}

void trie_set_parent_by_dfs(trie_ptr self, size_t current, size_t parent) {
  trie_node_ptr pNode = trie_access_node(self, current);
  pNode->trie_parent = parent;
  if (pNode->trie_child != 0)
    trie_set_parent_by_dfs(self, pNode->trie_child, current);
  if (pNode->trie_brother != 0)
    trie_set_parent_by_dfs(self, pNode->trie_brother, parent);
}

void trie_rebuild_parent_relation(trie_ptr self) {
  if (self->root->trie_child != 0)
    trie_set_parent_by_dfs(self, self->root->trie_child, 0);
  fprintf(stderr, "rebuild parent succeed!\n");
}

void trie_construct_automation(trie_ptr self) {
  size_t index;
  trie_node_ptr pNode = self->root;
  size_t iChild = pNode->trie_child;
  while (iChild != 0) {
    trie_node_ptr pChild = trie_access_node_export(self, iChild);
    iChild = pChild->trie_brother;
    pChild->trie_failed = 0; /* 设置 failed 域 */
  }

  for (index = 1; index < self->_autoindex; index++) { // bfs
    pNode = trie_access_node(self, index);
    iChild = pNode->trie_child;
    while (iChild != 0) {
      trie_node_ptr pChild = trie_access_node(self, iChild);
      unsigned char key = pChild->key;

      size_t iFailed = pNode->trie_failed;
      size_t match = trie_next_state_by_binary(self, iFailed, key);
      while (iFailed != 0 && match == 0) {
        iFailed = trie_access_node(self, iFailed)->trie_failed;
        match = trie_next_state_by_binary(self, iFailed, key);
      }
      iChild = pChild->trie_brother;
      pChild->trie_failed = match;
    }
  }
  fprintf(stderr, "construct AC automation succeed!\n");
}

void trie_destruct(trie_ptr p) {
  if (p != NULL) {
    int i;
    dict_release(p->_dict);
    for (i = 0; i < POOL_REGION_SIZE; i++) {
      if (p->_nodepool[i] != NULL)
        free(p->_nodepool[i]);
    }
    free(p);
  }
}

trie_ptr trie_construct(match_dict_ptr dict, bool enable_automation) {
#ifdef DEBUG
  long long t0, t1, t2, t3, t4;
#endif
  trie_ptr prime_trie = trie_alloc();
  if (prime_trie == NULL) return NULL;

  prime_trie->_dict = dict_assign(dict);
#ifdef DEBUG
  t0 = system_millisecond();
#endif
  for (size_t i = 0; i < dict->idx_count; i++) {
    match_dict_index_ptr index = &dict->index[i];
    if (!trie_add_keyword(prime_trie,
                          (const unsigned char *) index->keyword,
                          index->length,
                          index)) {
      fprintf(stderr, "fatal: encounter error when add keywords.\n");
      trie_destruct(prime_trie);
      prime_trie = NULL;
      break;
    }
  }
#ifdef DEBUG
  t1 = system_millisecond();
  fprintf(stderr, "s1: %ld ms\n", t1 - t0);
#endif
  if (prime_trie != NULL) {
    trie_sort_to_line(prime_trie);  /* sort node for bfs and binary-search */
#ifdef DEBUG
    t2 = system_millisecond();
    fprintf(stderr, "s2: %ld ms\n", t2 - t1);
#endif
    if (enable_automation) {
      trie_construct_automation(prime_trie);        /* 构建自动机 */
#ifdef DEBUG
      t3 = system_millisecond();
      fprintf(stderr, "s3: %ld ms\n", t3 - t2);
#endif
    }
  }
  return prime_trie;
}

trie_ptr trie_construct_by_file(const char *path, bool enable_automation) {
  trie_ptr prime_trie = NULL;
  match_dict_ptr dict = NULL;
  FILE *fp = NULL;

  if (path == NULL) {
    return NULL;
  }

  fp = fopen(path, "rb");
  if (fp == NULL) return NULL;

  dict = dict_alloc();
  if (dict == NULL) return NULL;

  if (dict_parser_by_file(dict, fp)) {
    prime_trie = trie_construct(dict, enable_automation);
  }

  fclose(fp);
  dict_release(dict);

  fprintf(stderr,
          "construct trie %s!\n",
          prime_trie != NULL ? "success" : "failed");
  return prime_trie;
}

trie_ptr trie_construct_by_s(const char *s, bool enable_automation) {
  trie_ptr prime_trie = NULL;
  match_dict_ptr dict = NULL;

  if (s == NULL) {
    return NULL;
  }

  dict = dict_alloc();
  if (dict == NULL) return NULL;

  if (dict_parser_by_s(dict, s)) {
    prime_trie = trie_construct(dict, enable_automation);
  }

  dict_release(dict);

  fprintf(stderr,
          "construct trie %s!\n",
          prime_trie != NULL ? "success" : "failed");
  return prime_trie;
}

void trie_ac_match(trie_ptr self, unsigned char content[], size_t len) {
  size_t i;
  trie_node_ptr pCursor = self->root;
  for (i = 0; i < len; ++i) {
    trie_node_ptr pNext =
        trie_next_node_by_binary(self, pCursor, content[i]);
    while (pCursor != self->root && pNext == self->root) {
      pCursor = trie_access_node(self, pCursor->trie_failed);
      pNext = trie_next_node_by_binary(self, pCursor, content[i]);
    }
    pCursor = pNext;
    while (pNext != self->root) {
      if (pNext->trie_dictidx != NULL)
        printf("%s\n", pNext->trie_dictidx->keyword);
      pNext = trie_access_node(self, pNext->trie_failed);
    }
  }
}

