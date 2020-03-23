/**
 * actrie.c
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#include "actrie.h"

// Prime Trie
// ========================================================

size_t trie_size(trie_t self) {
  return segarray_size(self->node_array);
}

static size_t trie_alloc_node(trie_t self) {
  if (1 == segarray_extend(self->node_array, 1)) {
    return segarray_size(self->node_array) - 1;
  } else {
    return (size_t)-1;
  }
}

void* trie_add_keyword(trie_t self, const char* keyword, size_t len, void* value) {
  const uint8_t* keys = (const uint8_t*)keyword;
  trie_node_t pNode = self->root;
  size_t iNode = 0; /* iParent 保存 pNode 的 index */
  size_t i = 0;

  for (i = 0; i < len; ++i) {
    /* 当创建时，使用插入排序的方法，以保证子节点链接关系有序 */
    size_t iChild = pNode->trie_child, iBrother = 0;  // iBrother 跟踪 iChild
    trie_node_t pChild = NULL;
    while (iChild != 0) {
      /* 从所有孩子中查找 */
      pChild = trie_access_node(self, iChild);
      if (pChild->key >= keys[i]) {
        break;
      }
      iBrother = iChild;
      iChild = pChild->trie_brother;
    }

    if (iChild != 0 && pChild->key == keys[i]) {
      /* 找到 */
      iNode = iChild;
      pNode = pChild;
    } else {
      /* 没找到, 创建. */
      size_t idx = trie_alloc_node(self);
      trie_node_t pc = NULL;

      if (idx == -1) {
        fprintf(stderr, "trie: alloc node failed.\nexit.\n");
        exit(-1);
      }

      pc = trie_access_node(self, idx);
      pc->key = keys[i];

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
        } else if (pChild->key < keys[i]) {
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

  void* old = pNode->value;
  pNode->value = value;

  return old;
}

size_t trie_next_state_by_binary(trie_t self, size_t iNode, uint8_t key) {
  trie_node_t pNode = trie_access_node(self, iNode);
  if (pNode->len >= 1) {
    size_t left = pNode->trie_child;
    size_t right = left + pNode->len - 1;
    if (key < trie_access_node(self, left)->key || trie_access_node(self, right)->key < key) {
      return 0;
    }
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

trie_node_t trie_next_node_by_binary(trie_t self, trie_node_t pNode, uint8_t key) {
  size_t left, right;
  if (pNode->len == 0 || key < trie_access_node(self, pNode->trie_child)->key ||
      trie_access_node(self, pNode->trie_child + pNode->len - 1)->key < key) {
    return self->root;
  }

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
  alib_swap(size_t, pa->trie_child, pb->trie_child);
  alib_swap(size_t, pa->trie_brother, pb->trie_brother);
  alib_swap(size_t, pa->trie_parent, pb->trie_parent);

  // dict index
  alib_swap(void*, pa->value, pb->value);

  alib_swap(int16_t, pa->len, pb->len);
  alib_swap(uint8_t, pa->key, pb->key);
}

// swap and return iChild's brother node
size_t trie_swap_node(trie_t self, size_t iChild, size_t iTarget) {
  trie_node_t pChild = trie_access_node(self, iChild);
  if (iChild != iTarget) {
    trie_node_t pTarget = trie_access_node(self, iTarget);

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
    alib_swap(trie_node_t, pChild, pTarget);

    /* 考虑上下级节点交换
     * 调整target的上级，child的下级
     */
    if (ipt == iChild) {
      /* target是child的下级，child是target的上级 */
      pTarget->trie_parent = iTarget;
      if (bct) {  // 链表头
        pChild->trie_child = iChild;
        if (ibc != 0) {
          trie_access_node(self, ibc)->trie_parent = iTarget;
        }
      } else {
        pChild->trie_brother = iChild;
        if (icc != 0) {
          trie_access_node(self, icc)->trie_parent = iTarget;
        }
      }
    } else {
      if (bct) {  // 链表头
        trie_access_node(self, ipt)->trie_child = iChild;
      } else {
        trie_access_node(self, ipt)->trie_brother = iChild;
      }
      if (icc != 0) {
        trie_access_node(self, icc)->trie_parent = iTarget;
      }
      if (ibc != 0) {
        trie_access_node(self, ibc)->trie_parent = iTarget;
      }
    }

    /* 调整直接上级指针 */
    if (bcc) {
      trie_access_node(self, ipc)->trie_child = iTarget;
    } else {
      trie_access_node(self, ipc)->trie_brother = iTarget;
    }

    /* 调整下级指针 */
    if (ict != 0) {
      trie_access_node(self, ict)->trie_parent = iChild;
    }
    if (ibt != 0) {
      trie_access_node(self, ibt)->trie_parent = iChild;
    }
  }
  return pChild->trie_brother;
}

void trie_free(trie_t trie, trie_node_free_f node_free_func) {
  if (trie != NULL) {
    if (node_free_func != NULL) {
      for (size_t i = 0; i < trie_size(trie); i++) {
        trie_node_t node = trie_access_node(trie, i);
        if (node->value != NULL) {
          node_free_func(trie, node->value);
        }
      }
    }
    segarray_destruct(trie->node_array);
    afree(trie);
  }
}

trie_t trie_alloc() {
  size_t root;
  trie_t trie = NULL;

  do {
    trie = (trie_t)amalloc(sizeof(trie_s));
    if (trie == NULL) {
      break;
    }

    trie->node_array = segarray_construct_with_type(trie_node_s);
    if (trie->node_array == NULL) {
      break;
    }

    root = trie_alloc_node(trie);
    if (root == (size_t)-1) {
      break;
    }

    trie->root = trie_access_node(trie, root);
    if (trie->root == NULL) {
      break;
    }

    return trie;
  } while (0);

  trie_free(trie, NULL);

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
       * 建树时的插入排序担保兄弟节点不会交换，且在重排后是稳定的
       */
      iChild = trie_swap_node(self, iChild, iTarget);
      iTarget++;
    }
  }
}

void trie_set_parent_by_dfs(trie_t self, size_t current, size_t parent) {
  trie_node_t pNode = trie_access_node(self, current);
  pNode->trie_parent = parent;
  if (pNode->trie_child != 0) {
    trie_set_parent_by_dfs(self, pNode->trie_child, current);
  }
  if (pNode->trie_brother != 0) {
    trie_set_parent_by_dfs(self, pNode->trie_brother, parent);
  }
}

void trie_rebuild_parent_relation(trie_t self) {
  if (self->root->trie_child != 0) {
    trie_set_parent_by_dfs(self, self->root->trie_child, 0);
  }
}

void trie_build_automation(trie_t self) {
  size_t index;
  trie_node_t pNode = self->root;
  size_t iChild = pNode->trie_child;
  while (iChild != 0) {
    trie_node_t pChild = trie_access_node(self, iChild);
    iChild = pChild->trie_brother;
    pChild->trie_failed = 0; /* 设置 failed 域 */
  }

  size_t size = trie_size(self);
  for (index = 1; index < size; index++) {  // bfs
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
}

void* trie_search(trie_t self, const char* keyword, size_t len) {
  const uint8_t* keys = (const uint8_t*)keyword;
  trie_node_t pNode = self->root;
  size_t i = 0;

  for (i = 0; i < len; ++i) {
    /* 当创建时，使用插入排序的方法，以保证子节点链接关系有序 */
    size_t iChild = pNode->trie_child;  // iBrother 跟踪 iChild
    trie_node_t pChild = NULL;
    while (iChild != 0) {
      /* 从所有孩子中查找 */
      pChild = trie_access_node(self, iChild);
      if (pChild->key >= keys[i]) {
        break;
      }
      iChild = pChild->trie_brother;
    }

    if (iChild != 0 && pChild->key == keys[i]) {
      /* 找到 */
      pNode = pChild;
    } else {
      return NULL;
    }
  }

  return pNode->value;
}
