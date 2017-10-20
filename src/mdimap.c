//
// Created by james on 10/17/17.
//

#include "mdimap.h"

sptr_t mdimap_compare(avl_node_t node, void *key) {
  mdim_node_t n = container_of(node, mdim_node_s, avl_elem);
  return (sptr_t) n->idx->_tag - (sptr_t) key;
}

avl_node_t mdimap_replace(avl_node_t old, avl_node_t new) {
  if (new != NULL) {
    mdim_node_t new_node = container_of(new, mdim_node_s, avl_elem);
    if (old != NULL) { // insert head
      mdim_node_t old_node = container_of(old, mdim_node_s, avl_elem);
      old_node->last = new_node;
      new_node->next = old_node;
      new_node->last = NULL;
    } else { // new list
      new_node->next = new_node->last = NULL;
    }
  }
  return new;
}

avl_node_t mdimap_append(avl_node_t old, avl_node_t new) {
  if (new != NULL) {
    mdim_node_t new_node = container_of(new, mdim_node_s, avl_elem);
    if (old != NULL) { // insert head
      mdim_node_t old_node = container_of(old, mdim_node_s, avl_elem);
      while (old_node->next != NULL) // go to tail
        old_node = old_node->next;
      old_node->next = new_node;
      new_node->last = old_node;
      new_node->next = NULL;
      return old;
    } else { // new list
      new_node->next = new_node->last = NULL;
    }
  }
  return new;
}

mdimap_t mdimap_construct(bool replace) {
  mdimap_t map = NULL;
  do {
    map = avl_construct(mdimap_compare);
    if (map == NULL) break;

    if (replace) {
      map->replace = mdimap_replace;
    } else {
      map->replace = mdimap_append;
    }

    return map;
  } while (0);

  mdimap_destruct(map);

  return NULL;
}

bool mdimap_destruct(mdimap_t map) {
  return avl_destruct(map);
}

bool mdimap_reset(mdimap_t map) {
  avl_reset(map);
  return true;
}

mdim_node_t mdimap_search(mdimap_t map, void *key) {
  avl_node_t node = avl_search(map, key);
  return node == NULL ? NULL : container_of(node, mdim_node_s, avl_elem);
}

bool mdimap_insert(mdimap_t map, mdim_node_t node) {
  avl_insert(map, node->idx->_tag, &node->avl_elem);
  return true;
}

bool mdimap_delete(mdimap_t map, mdim_node_t node) {
  if (node->last != NULL) { // not list head
    node->last->next = node->next;
    if (node->next != NULL)
      node->next->last = node;
  } else {
    if (node->next == NULL) {
      avl_delete(map, node->idx->_tag);
    } else {
      node->next->last = NULL;
      avl_replace(map, node->idx->_tag, &node->next->avl_elem);
    }
  }
  return true;
}
