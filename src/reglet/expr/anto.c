/**
 * anto.c
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#include "anto.h"

typedef struct _expression_anti_antonym_context_ {
  expr_ctx_s header;
  avl_t antonym_cache;
  deque_node_s center_queue[1];
} anto_ctx_s, *anto_ctx_t;

void anto_ctx_free(expr_ctx_t expr_ctx, reg_ctx_t reg_ctx) {
  anto_ctx_t anto_ctx = container_of(expr_ctx, anto_ctx_s, header);

  if (reg_ctx->reset_or_free) {
    avl_walk_in_order(anto_ctx->antonym_cache, NULL, free_pos_cache, NULL, reg_ctx);
  }

  avl_destruct(anto_ctx->antonym_cache);

  pos_cache_t center = deque_pop_front(anto_ctx->center_queue, pos_cache_s, embed.deque_elem);
  while (center != NULL) {
    dynapool_free_node(reg_ctx->pos_cache_pool, center);
    center = deque_pop_front(anto_ctx->center_queue, pos_cache_s, embed.deque_elem);
  }

  afree(anto_ctx);
}

void expr_activate_anto_ctx(expr_ctx_t expr_ctx, reg_ctx_t context);

anto_ctx_t anto_ctx_alloc(expr_anto_t expr_anto) {
  anto_ctx_t anto_ctx = amalloc(sizeof(anto_ctx_s));
  expr_ctx_init(&anto_ctx->header, &expr_anto->header, anto_ctx_free, expr_activate_anto_ctx);
  anto_ctx->antonym_cache = avl_construct(pos_cache_cmp_eoso);
  deque_init(anto_ctx->center_queue);
  return anto_ctx;
}

void expr_init_anto(expr_anto_t self, expr_t target, expr_feed_f feed) {
  expr_init(&self->header, target, feed);
}

void expr_feed_anto_antonym(expr_t expr, pos_cache_t antonym, reg_ctx_t context) {
  expr_anto_t self = container_of(expr, expr_anto_s, header);

  anto_ctx_t anto_ctx;
  avl_node_t node = avl_search(context->expr_ctx_map, expr);
  if (node == NULL) {
    anto_ctx = anto_ctx_alloc(self);
    avl_insert(context->expr_ctx_map, expr, &anto_ctx->header.avl_elem);
  } else {
    anto_ctx = container_of(node, anto_ctx_s, header.avl_elem);
  }

  avl_insert(anto_ctx->antonym_cache, &antonym->pos, &antonym->embed.avl_elem);
}

void expr_feed_anto_center(expr_t expr, pos_cache_t center, reg_ctx_t context) {
  expr_anto_t self = container_of(expr, expr_anto_s, header);

  anto_ctx_t anto_ctx = NULL;
  avl_node_t node = avl_search(context->expr_ctx_map, expr);
  if (node == NULL) {
    anto_ctx = anto_ctx_alloc(self);
    avl_insert(context->expr_ctx_map, expr, &anto_ctx->header.avl_elem);
  } else {
    anto_ctx = container_of(node, anto_ctx_s, header.avl_elem);
  }

  if (avl_search_ext(anto_ctx->antonym_cache, &center->pos.so, pos_cache_eq_eo) == NULL) {
    if (deque_empty(anto_ctx->center_queue)) {
      prique_push(context->activate_queue, &anto_ctx->header);
    }
    deque_push_back(anto_ctx->center_queue, center, pos_cache_s, embed.deque_elem);
  }
}

void expr_activate_anto_ctx(expr_ctx_t expr_ctx, reg_ctx_t context) {
  anto_ctx_t anto_ctx = container_of(expr_ctx, anto_ctx_s, header);
  pos_cache_t center = deque_pop_front(anto_ctx->center_queue, pos_cache_s, embed.deque_elem);
  while (center != NULL) {
    if (avl_search_ext(anto_ctx->antonym_cache, &center->pos.so, pos_cache_eq_eo) == NULL) {
      expr_feed_target(anto_ctx->header.expr, center, context);
    } else {
      dynapool_free_node(context->pos_cache_pool, center);
    }
    center = deque_pop_front(anto_ctx->center_queue, pos_cache_s, embed.deque_elem);
  }
}
