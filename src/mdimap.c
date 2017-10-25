//
// Created by james on 10/17/17.
//

#include "mdimap.h"

size_t size_rank[] = {
    /*31, 67,*/ 127, 257, 509, 1021, 2053, 4099,
    8191, 16381, 32771, 65537, 131071, 262147
};

size_t size_threshold[] = {
    /*25, 54,*/ 102, 205, 407, 817, 1745, 3480,
    6962, 13924, 29494, 58982, 117964, SIZE_MAX
};

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

static inline uptr_t mdimap_hash(void *key) {
  return (uptr_t) key / sizeof(mdi_s);
}

mdimap_t mdimap_construct(bool replace) {
  mdimap_t map = NULL;
  do {
    map = amalloc(sizeof(mdimap_s));
    if (map == NULL) break;

    map->rehash = false;
    map->replace = replace;
    map->rank = map->size = map->len = 0;
    map->bucket = amalloc(sizeof(mdim_bucket_s) * size_rank[0]);
    if (map->bucket == NULL) break;

    memset(map->bucket, 0, sizeof(mdim_bucket_s) * size_rank[0]);

    return map;
  } while (0);

  mdimap_destruct(map);

  return NULL;
}

void mdimap_clean_bucket(mdim_bucket_t bucket, size_t size) {
  if (bucket == NULL || size == 0) return;
  for (size_t i = 0; i < size; i++) {
    if (bucket[i].type == mdim_bucket_type_avl)
      avl_destruct(bucket[i].store.avl);
  }
  afree(bucket);
}

bool mdimap_destruct(mdimap_t map) {
  if (map == NULL) return false;
  mdimap_clean_bucket(map->bucket, size_rank[map->rank]);
  afree(map);
  return true;
}

bool mdimap_reset(mdimap_t map) {
  if (map == NULL) return false;
  if (map->len == 0 && map->size == 0) return true;
  for (size_t i = 0; i < size_rank[map->rank]; i++) {
    if (map->bucket[i].type == mdim_bucket_type_avl) {
      avl_destruct(map->bucket[i].store.avl);
//      avl_reset(map->bucket[i].store.avl);
    }
    map->bucket[i].type = mdim_bucket_type_empty;
  }
  map->size = map->len = 0;
  return true;
}

bool mdimap_insert_without_extend(mdimap_t map, mdim_node_t node) {
  uptr_t hash = mdimap_hash(node->idx->_tag);
  size_t index = hash % size_rank[map->rank];
  mdim_bucket_t bucket = map->bucket + index;
  if (bucket->type == mdim_bucket_type_empty) {
    // direct store
    bucket->store.data = node;
    if (!map->rehash) {
      node->next = node->last = NULL;
    }
    bucket->type = mdim_bucket_type_direct;
    map->size++;
  } else if (bucket->type == mdim_bucket_type_direct) {
    mdim_node_t old = bucket->store.data;
    if (old->idx->_tag == node->idx->_tag) {
      // insert link
      if (map->replace) {
        old->last = node;
        node->next = old;
        node->last = NULL;
        bucket->store.data = node;
      } else {
        while (old->next != NULL)
          old = old->next;
        old->next = node;
        node->last = old;
        node->next = NULL;
      }
    } else {
      // store in avl
      avl_t avl = avl_construct(mdimap_compare);
      if (avl == NULL) return false;
      avl_insert(avl, old->idx->_tag, &old->avl_elem);
      if (!map->rehash) {
        if (map->replace) {
          avl->replace = mdimap_replace;
        } else {
          avl->replace = mdimap_append;
        }
      }
      avl_insert(avl, node->idx->_tag, &node->avl_elem);
      bucket->store.avl = avl;
      bucket->type = mdim_bucket_type_avl;
      map->len++;
    }
  } else {
    // avl
    avl_t avl = bucket->store.avl;
    avl_insert(avl, node->idx->_tag, &node->avl_elem);
    if (avl->len == 1)
      map->size++;
  }
  return true;
}

mdim_node_t mdimap_search(mdimap_t map, void *key) {
  uptr_t hash = mdimap_hash(key);
  size_t index = hash % size_rank[map->rank];
  mdim_bucket_t bucket = map->bucket + index;
  if (bucket->type == mdim_bucket_type_direct) {
    if (bucket->store.data->idx->_tag == key)
      return bucket->store.data;
  } else if (bucket->type == mdim_bucket_type_avl) {
    avl_t avl = bucket->store.avl;
    avl_node_t node = avl_search(avl, key);
    if (node != NULL)
      return container_of(node, mdim_node_s, avl_elem);
  }
  return NULL;
}

void mdimap_avl_dfs(mdimap_t map, avl_node_t node) {
  if (node == NULL) return;
  mdimap_avl_dfs(map, node->avl_left);
  mdimap_avl_dfs(map, node->avl_right);
  mdimap_insert_without_extend(map, container_of(node, mdim_node_s, avl_elem));
}

bool mdimap_rehash_bucket(mdimap_t map, mdim_bucket_t new, size_t nrank) {
  mdim_bucket_t old = map->bucket;
  size_t osize = size_rank[map->rank];
  map->bucket = new;
  map->rank = nrank;
  map->size = map->len = 0;

  map->rehash = true;

  memset(new, 0, sizeof(mdim_bucket_s) * size_rank[map->rank]);
  for (size_t i = 0; i < osize; i++) {
    if (old[i].type == mdim_bucket_type_direct)
      mdimap_insert_without_extend(map, old[i].store.data);
    else if (old[i].type == mdim_bucket_type_avl)
      mdimap_avl_dfs(map, old[i].store.avl->root);
  }
  mdimap_clean_bucket(old, osize);

  for (size_t i = 0; i < size_rank[map->rank]; i++) {
    if (map->bucket[i].type == mdim_bucket_type_avl) {
      if (map->replace) {
        map->bucket[i].store.avl->replace = mdimap_replace;
      } else {
        map->bucket[i].store.avl->replace = mdimap_append;
      }
    }
  }
  map->rehash = false;

  return true;
}

bool mdimap_insert(mdimap_t map, mdim_node_t node) {
  if (map->size >= size_threshold[map->rank]) {
    // extend
    mdim_bucket_s *bucket = amalloc(sizeof(mdim_bucket_s) * size_rank[map->rank + 1]);
    if (bucket != NULL) {
      // re-hash
      mdimap_rehash_bucket(map, bucket, map->rank + 1);
    }
  }

  return mdimap_insert_without_extend(map, node);
}

bool mdimap_delete(mdimap_t map, mdim_node_t node) {
  if (node->last != NULL) { // not list head
    node->last->next = node->next;
    if (node->next != NULL)
      node->next->last = node;
  } else {
    uptr_t hash = mdimap_hash(node->idx->_tag);
    size_t index = hash % size_rank[map->rank];
    mdim_bucket_t bucket = map->bucket + index;
    if (bucket->type == mdim_bucket_type_direct) {
      if (node->next == NULL) {
        bucket->type = mdim_bucket_type_empty;
        map->size--;
      } else {
        node->next->last = NULL;
        bucket->store.data = node->next;
      }
    } else {
      avl_t avl = map->bucket[index].store.avl;
      if (node->next == NULL) {
        avl_delete(avl, node->idx->_tag);
        if (avl->len == 0)
          map->size--;
      } else {
        node->next->last = NULL;
        avl_replace(avl, node->idx->_tag, &node->next->avl_elem);
      }
    }
  }
  return true;
}
