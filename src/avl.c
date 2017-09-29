//
// Created by james on 9/28/17.
//
// reference:
//    http://adtinfo.org/libavl.html/AVL-Trees.html
//

#include "avl.h"

const static size_t avl_extend_space_size = ROUND_UP_16(AVL_EXTEND_SPACE_SIZE);

int avl_default_cmp_data(void *a, void *b) {
  return (int) a - (int) b;
}

void avl_default_set_data(void **container, void *data) {
  *(int *) container = (int) data;
}

#if AVL_USER_INDEX
#define avl_access_node(self, index) (self->_nodepool + index)
#else
#define avl_access_node(self, index) index
#endif

bool avl_init(avl_t self, size_t size) {
  if (self == NULL) return false;

  self->_len = 0;

  if (size == 0) {
    size = avl_extend_space_size;
  }

  // size 为 16 的倍数
  size = ROUND_UP_16(size);
  self->_nodepool = malloc(sizeof(avl_node_s) * size);
  if (self->_nodepool == NULL) return false;
  self->_size = size;

  // free list
  avl_node_s *p = &self->_nodepool[0];
  for (size_t i = 0; i < size; i++) {
#if AVL_USER_INDEX
    p->right = i + 1;
    p->left = i - 1;
#else
    p->right = p + 1;
    p->left = p - 1;
#endif
    p->data = NULL;
    p++;
  }

#if AVL_USER_INDEX
  self->avl_root = 0;

  avl_free_leader(self).right = 2;
  avl_free_leader(self).left = size - 1;
#else
  self->avl_root = avl_invalid_node(self);

  avl_free_leader(self).right = &self->_nodepool[2];
  avl_free_leader(self).left = &self->_nodepool[size - 1];
#endif
  avl_access_node(self, avl_free_leader(self).right)->left =
      avl_free_leader_addr(self);
  avl_access_node(self, avl_free_leader(self).left)->right =
      avl_free_leader_addr(self);

  self->cmp_data = avl_default_cmp_data;
  self->set_data = avl_default_set_data;

  return true;
}

avl_node_t avl_alloc_node(avl_t self, avl_node_t *pp) {
  if (avl_free_next(self) == avl_free_empty(self)) {
    // extend memory
    size_t new_size = self->_size + avl_extend_space_size;
    avl_node_s *new_pool = malloc(sizeof(avl_node_s) * new_size);
    if (new_pool == NULL) return NULL;

    // copy old node pool
    avl_node_s *p = &new_pool[0], *op = &self->_nodepool[0];
    for (size_t i = 0; i < self->_size; i++) {
      p->data = op->data;
      p->bf = op->bf;
#if AVL_USER_INDEX
      p->parent = op->parent;
      p->right = op->right;
      p->left = op->left;
#else
      p->parent = new_pool + (op->parent - self->_nodepool);
      p->right = new_pool + (op->right - self->_nodepool);
      p->left = new_pool + (op->left - self->_nodepool);
#endif
      p++;
      op++;
    }

    avl_node_s *np = p;

    // init new node pool
    for (size_t i = 0; i < avl_extend_space_size; i++) {
      p->data = NULL;
#if AVL_USER_INDEX
      p->right = self->_size + i + 1;
      p->left = self->_size + i - 1;
#else
      p->right = p + 1;
      p->left = p - 1;
#endif
      p++;
    }

#if !AVL_USER_INDEX
    *pp = new_pool + (*pp - self->_nodepool);
#endif

    // switch to new node pool
    free(self->_nodepool);
    self->_nodepool = new_pool;

#if AVL_USER_INDEX
    avl_free_leader(self).right = self->_size;
    avl_free_leader(self).left = new_size - 1;
#else
    avl_free_leader(self).right = np;
    avl_free_leader(self).left = p - 1;
#endif
    avl_access_node(self, avl_free_leader(self).right)->left =
        avl_free_leader_addr(self);
    avl_access_node(self, avl_free_leader(self).left)->right =
        avl_free_leader_addr(self);

    self->_size = new_size;
  }

  // remove node from free list
  avl_node_t p = avl_free_next(self);
  avl_access_node(self, avl_access_node(self, p)->left)->right =
      avl_access_node(self, p)->right;
  avl_access_node(self, avl_access_node(self, p)->right)->left =
      avl_access_node(self, p)->left;

  return p;
}

void avl_remove_all(avl_t self) {
  avl_node_s *p = &self->_nodepool[2];
  for (size_t i = 0; i < self->_len; i++) {
#if AVL_USER_INDEX
    p->right = i + 3;
    p->left = i + 1;
#else
    p->right = p + 1;
    p->left = p - 1;
#endif
    p->data = NULL;
    p++;
  }

#if AVL_USER_INDEX
  if (avl_free_next(self) != avl_free_empty(self)) {
    p->left = self->_len + 1;
  }

  self->avl_root = 0;

  avl_free_leader(self).right = 2;
  avl_free_leader(self).left = self->_size - 1;
#else
  if (avl_free_next(self) != avl_free_empty(self)) {
    p->left = p - 1;
  }

  self->avl_root = avl_invalid_node(self);

  avl_free_leader(self).right = &self->_nodepool[2];
  avl_free_leader(self).left = &self->_nodepool[size - 1];
#endif

  self->_len = 0;
}

/*
 *         a                 b
 *       +   +             +   +
 *     b       c  =>     d       a
 *   +   +                     +   +
 * d       e                 e       c
 */
avl_node_t avl_rotate_ll(avl_t self, avl_node_t a) {
  avl_node_t p = avl_access_node(self, a)->parent;
  avl_node_t b = avl_access_node(self, a)->left;
  avl_node_t e = avl_access_node(self, b)->right;

  avl_access_node(self, b)->parent = p;
  avl_access_node(self, b)->right = a;

  avl_access_node(self, a)->parent = b;
  avl_access_node(self, a)->left = e;

  avl_access_node(self, e)->parent = a;

  avl_access_node(self, a)->bf = 0;
  avl_access_node(self, b)->bf = 0;

  return b;
}

/*
 *         a
 *       +   +                   e
 *     b       c              +     +
 *   +   +         =>      b          a
 * d       e             +   +      +   +
 *       +   +         d       f   g      c
 *     f       g
 */
avl_node_t avl_rotate_lr(avl_t self, avl_node_t a) {
  avl_node_t p = avl_access_node(self, a)->parent;
  avl_node_t b = avl_access_node(self, a)->left;
  avl_node_t e = avl_access_node(self, b)->right;
  avl_node_t f = avl_access_node(self, e)->left;
  avl_node_t g = avl_access_node(self, e)->right;

  avl_access_node(self, e)->left = b;
  avl_access_node(self, e)->right = a;
  avl_access_node(self, e)->parent = p;

  avl_access_node(self, b)->parent = e;
  avl_access_node(self, b)->right = f;

  avl_access_node(self, a)->parent = e;
  avl_access_node(self, a)->left = g;

  avl_access_node(self, f)->parent = b;

  avl_access_node(self, g)->parent = a;

  if (avl_access_node(self, e)->bf >= 0) {
    avl_access_node(self, a)->bf = -1;
    avl_access_node(self, b)->bf = 0;
  } else {
    avl_access_node(self, a)->bf = 0;
    avl_access_node(self, b)->bf = 1;
  }
  avl_access_node(self, e)->bf = 0;

  return e;
}

avl_node_t avl_rotate_rr(avl_t self, avl_node_t a) {
  avl_node_t p = avl_access_node(self, a)->parent;
  avl_node_t b = avl_access_node(self, a)->right;
  avl_node_t e = avl_access_node(self, b)->left;

  avl_access_node(self, b)->parent = p;
  avl_access_node(self, b)->left = a;

  avl_access_node(self, a)->parent = b;
  avl_access_node(self, a)->right = e;

  avl_access_node(self, e)->parent = a;

  avl_access_node(self, a)->bf = 0;
  avl_access_node(self, b)->bf = 0;

  return b;
}

avl_node_t avl_rotate_rl(avl_t self, avl_node_t a) {
  avl_node_t p = avl_access_node(self, a)->parent;
  avl_node_t b = avl_access_node(self, a)->right;
  avl_node_t e = avl_access_node(self, b)->left;
  avl_node_t f = avl_access_node(self, e)->right;
  avl_node_t g = avl_access_node(self, e)->left;

  avl_access_node(self, e)->right = b;
  avl_access_node(self, e)->left = a;
  avl_access_node(self, e)->parent = p;

  avl_access_node(self, b)->parent = e;
  avl_access_node(self, b)->left = f;

  avl_access_node(self, a)->parent = e;
  avl_access_node(self, a)->right = g;

  avl_access_node(self, f)->parent = b;

  avl_access_node(self, g)->parent = a;

  if (avl_access_node(self, e)->bf <= 0) {
    avl_access_node(self, a)->bf = 1;
    avl_access_node(self, b)->bf = 0;
  } else {
    avl_access_node(self, a)->bf = 0;
    avl_access_node(self, b)->bf = -1;
  }
  avl_access_node(self, e)->bf = 0;

  return e;
}

bool avl_insert(avl_t self, void *data) {
  avl_node_t *pc;
  avl_node_t p = self->avl_root, pp = avl_invalid_node(self);
  ll path = 1LL;
  while (p != avl_invalid_node(self)) {
    int ret = self->cmp_data(avl_access_node(self, p)->data, data);
    if (ret == 0) {
      break;
    } else {
      pp = p;
      path <<= 1;
      if (ret > 0) {
        p = avl_access_node(self, p)->left;
      } else {
        p = avl_access_node(self, p)->right;
        path |= 1LL;
      }
    }
  }

  if (p != avl_invalid_node(self)) { // hit
    self->set_data(&avl_access_node(self, p)->data, data);
  } else { // new node
    p = avl_alloc_node(self, &pp);
    avl_access_node(self, p)->parent = pp;
    avl_access_node(self, p)->left = avl_invalid_node(self);
    avl_access_node(self, p)->right = avl_invalid_node(self);
    avl_access_node(self, p)->bf = 0;
    self->set_data(&avl_access_node(self, p)->data, data);

    if (path & 1LL) {
      pc = &avl_access_node(self, pp)->right;
    } else {
      pc = &avl_access_node(self, pp)->left;
    }
    *pc = p;

    // look back, maintain
    while (pp != avl_invalid_node(self)) {
      avl_access_node(self, pp)->bf += path & 1LL ? 1 : -1;
      path >>= 1;
      if (avl_access_node(self, pp)->bf <= -2) {
        if (path & 1LL) {
          pc = &avl_access_node(self, avl_access_node(self, pp)->parent)->right;
        } else {
          pc = &avl_access_node(self, avl_access_node(self, pp)->parent)->left;
        }

        if (avl_access_node(self, avl_access_node(self, pp)->left)->bf <= 0) {
          *pc = avl_rotate_ll(self, pp);
        } else {
          *pc = avl_rotate_lr(self, pp);
        }
        break;
      } else if (avl_access_node(self, pp)->bf >= 2) {
        if (path & 1LL) {
          pc = &avl_access_node(self, avl_access_node(self, pp)->parent)->right;
        } else {
          pc = &avl_access_node(self, avl_access_node(self, pp)->parent)->left;
        }

        if (avl_access_node(self, avl_access_node(self, pp)->right)->bf <= 0) {
          *pc = avl_rotate_rl(self, pp);
        } else {
          *pc = avl_rotate_rr(self, pp);
        }
        return true;
      } else if (avl_access_node(self, pp)->bf == 0) {
        break;
      }
      pp = avl_access_node(self, pp)->parent;
    }

    self->_len++;
  }

  return true;
}

void *avl_search(avl_t self, void *data) {
  avl_node_t p = self->avl_root;
  while (p != avl_invalid_node(self)) {
    int ret = self->cmp_data(avl_access_node(self, p)->data, data);
    if (ret == 0) {
      break;
    } else {
      if (ret > 0) {
        p = avl_access_node(self, p)->left;
      } else {
        p = avl_access_node(self, p)->right;
      }
    }
  }

  if (p != avl_invalid_node(self)) { // hit
    return avl_access_node(self, p)->data;
  }

  return NULL;
}

//void main() {
//  avl_s avl;
//  avl_init(&avl, 0);
//  for (int i = 1; i < 20; i++) {
//    avl_insert(&avl, -i);
//  }
//  avl_remove_all(&avl);
//  printf("end!");
//}