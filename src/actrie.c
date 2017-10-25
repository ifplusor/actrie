#include "actrie.h"
#include "list.h"


// Prime Trie
// ========================================================

size_t trie_size(trie_t self) {
  return self->_autoindex;
}

static size_t trie_alloc_node(trie_t self) {
  size_t region = self->_autoindex >> REGION_OFFSET;
//	size_t position = self->_autoindex & POSITION_MASK;
#ifdef CHECK
  if (region >= POOL_REGION_SIZE)
      return (size_t)-1;
#endif // CHECK
  if (self->_nodepool[region] == NULL) {
    trie_node_t pnode =
        (trie_node_t) amalloc(sizeof(trie_node_s) * POOL_POSITION_SIZE);
    if (pnode == NULL) return (size_t) -1;
    self->_nodepool[region] = pnode;
    memset(pnode, 0, sizeof(trie_node_s) * POOL_POSITION_SIZE);
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

static inline trie_node_t trie_access_node(trie_t self, size_t index) {
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

trie_node_t trie_access_node_export(trie_t self, size_t index) {
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

bool trie_add_keyword(trie_t self, const unsigned char keyword[], size_t len, aobj obj) {
  trie_node_t pNode = self->root;
  size_t iNode = 0; /* iParent保存pNode的index */
  size_t i = 0;

  for (i = 0; i < len; ++i) {
    /* 当创建时，使用插入排序的方法，以保证子节点链接关系有序 */
    size_t iChild = pNode->trie_child, iBrother = 0; // iBrother跟踪iChild
    trie_node_t pChild = NULL;
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
      trie_node_t pc = NULL;

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
          trie_node_t pBrother = trie_access_node(self, iBrother);
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

  /* 头插法链接 dict_index */
  // NOTE: 注意内存泄漏
  aobj list = pNode->trie_idxlist;
  pNode->trie_idxlist = _(list, obj, cons, list);
  _release(list);

  return true;
}

size_t trie_next_state_by_binary(trie_t self, size_t iNode, unsigned char key) {
  trie_node_t pNode = trie_access_node(self, iNode);
  if (pNode->len >= 1) {
    size_t left = pNode->trie_child;
    size_t right = left + pNode->len - 1;
    if (key < trie_access_node(self, left)->key ||
        trie_access_node(self, right)->key < key)
      return 0;
    while (left <= right) {
      size_t middle = (left + right) >> 1;
      trie_node_t pMiddle = trie_access_node(self, middle);
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

trie_node_t trie_next_node_by_binary(trie_t self, trie_node_t pNode,
                                       unsigned char key) {
  size_t left, right;
  if (key < trie_access_node(self, pNode->trie_child)->key ||
      trie_access_node(self, pNode->trie_child + pNode->len - 1)->key < key)
    return self->root;

  left = pNode->trie_child;
  right = left + pNode->len - 1;
  while (left <= right) {
    size_t middle = (left + right) >> 1;
    trie_node_t pMiddle = trie_access_node(self, middle);
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

void trie_swap_node_data(trie_node_t pa, trie_node_t pb) {
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
size_t trie_swap_node(trie_t self, size_t iChild, size_t iTarget) {
  trie_node_t pChild = trie_access_node(self, iChild);
  if (iChild != iTarget) {
    trie_node_t ptmp, pTarget = trie_access_node(self, iTarget);

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

trie_t trie_alloc() {
  size_t root;
  int i;
  trie_t p = NULL;

  do {
    p = (trie_t) amalloc(sizeof(trie_s));
    if (p == NULL) break;

    p->_dict = NULL;
    for (i = 0; i < POOL_REGION_SIZE; i++)
      p->_nodepool[i] = NULL;
    p->_autoindex = 0;

    root = trie_alloc_node(p);
    if (root == (size_t) -1) break;

    p->root = trie_access_node(p, root);
    if (p->root == NULL) break;

    return p;
  } while (0);

  trie_destruct(p);

  return NULL;
}

void trie_sort_to_line(trie_t self) {
  size_t i, iTarget = 1;
  for (i = 0; i < iTarget; i++) { /* 隐式bfs队列 */
    trie_node_t pNode = trie_access_node(self, i);
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

void trie_set_parent_by_dfs(trie_t self, size_t current, size_t parent) {
  trie_node_t pNode = trie_access_node(self, current);
  pNode->trie_parent = parent;
  if (pNode->trie_child != 0)
    trie_set_parent_by_dfs(self, pNode->trie_child, current);
  if (pNode->trie_brother != 0)
    trie_set_parent_by_dfs(self, pNode->trie_brother, parent);
}

void trie_rebuild_parent_relation(trie_t self) {
  if (self->root->trie_child != 0)
    trie_set_parent_by_dfs(self, self->root->trie_child, 0);
  fprintf(stderr, "rebuild parent succeed!\n");
}

void trie_construct_automation(trie_t self) {
  size_t index;
  trie_node_t pNode = self->root;
  size_t iChild = pNode->trie_child;
  while (iChild != 0) {
    trie_node_t pChild = trie_access_node(self, iChild);
    iChild = pChild->trie_brother;
    pChild->trie_failed = 0; /* 设置 failed 域 */
  }

  for (index = 1; index < self->_autoindex; index++) { // bfs
    pNode = trie_access_node(self, index);
    iChild = pNode->trie_child;
    while (iChild != 0) {
      trie_node_t pChild = trie_access_node(self, iChild);
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

void trie_destruct(trie_t self) {
  if (self != NULL) {
    for (int i = 0; i < POOL_REGION_SIZE; i++) {
      if (self->_nodepool[i] != NULL) {
        for (int j = 0; j < POOL_POSITION_SIZE; j++) {
          _release(self->_nodepool[i][j].trie_idxlist);
        }
        afree(self->_nodepool[i]);
      }
    }
    dict_release(self->_dict);
    afree(self);
  }
}

trie_t trie_construct(match_dict_t dict, trie_config_t config) {
  trie_t prime_trie = NULL;

  do {
    prime_trie = trie_alloc();
    if (prime_trie == NULL) break;

    prime_trie->_dict = dict_retain(dict);

    size_t i = 0;
    for (; i < dict->idx_count; i++) {
      mdi_t index = &dict->index[i];
      if (mdi_prop_get_matcher(index->prop) != config->filter) continue;
      if (!trie_add_keyword(prime_trie, index->_keyword, index->length, index)) {
        fprintf(stderr, "%s(%d) - error: encounter error when add keywords!\n",
                __FILE__, __LINE__);
        break;
      }
    }
    if (i != dict->idx_count) break;

    trie_sort_to_line(prime_trie);  /* sort node for bfs and binary-search */

    if (config->enable_automation)
      trie_construct_automation(prime_trie);        /* 构建自动机 */

    return prime_trie;
  } while (0);

  trie_destruct(prime_trie);

  return NULL;
}

aobj trie_search(trie_t self, const unsigned char keyword[], size_t len) {
  trie_node_t pNode = self->root;
  size_t i = 0;

  for (i = 0; i < len; ++i) {
    /* 当创建时，使用插入排序的方法，以保证子节点链接关系有序 */
    size_t iChild = pNode->trie_child; // iBrother跟踪iChild
    trie_node_t pChild = NULL;
    while (iChild != 0) {
      /* 从所有孩子中查找 */
      pChild = trie_access_node(self, iChild);
      if (pChild->key >= keyword[i]) break;
      iChild = pChild->trie_brother;
    }

    if (iChild != 0 && pChild->key == keyword[i]) {
      /* 找到 */
      pNode = pChild;
    } else {
      return NULL;
    }
  }

  return pNode->trie_idxlist;
}