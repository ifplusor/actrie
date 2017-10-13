//
// Created by james on 10/13/17.
//

#include "dlnk.h"

void dlnk_init(dlnk_node_t sentinel) {
  sentinel->forw = sentinel->back = sentinel;
}

inline void dlnk_insert(dlnk_node_t point, dlnk_node_t node) {
  node->forw = point->forw;
  node->forw->back = node;
  node->back = point;
  point->forw = node;
}

inline void dlnk_delete(dlnk_node_t node) {
  node->forw->back = node->back;
  node->back->forw = node->forw;
}
