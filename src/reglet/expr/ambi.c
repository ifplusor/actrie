/**
 * ambi.c
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#include "ambi.h"

typedef struct _expression_anti_ambiguity_context_ {
  expr_ctx_s header;
  avl_t ambiguity_cache_eoso;
  avl_t ambiguity_cache_soeo;
  deque_node_s center_queue[1];
} ambi_ctx_s, *ambi_ctx_t;

void ambi_ctx_free(expr_ctx_t expr_ctx, reg_ctx_t reg_ctx) {
  ambi_ctx_t ambi_ctx = container_of(expr_ctx, ambi_ctx_s, header);

  if (reg_ctx->reset_or_free) {
    avl_walk_in_order(ambi_ctx->ambiguity_cache_eoso, NULL, free_pos_cache, NULL, reg_ctx);
    avl_walk_in_order(ambi_ctx->ambiguity_cache_soeo, NULL, free_pos_cache, NULL, reg_ctx);

    pos_cache_t pos_cache = deque_pop_front(ambi_ctx->center_queue, pos_cache_s, embed.deque_elem);
    while (pos_cache != NULL) {
      dynapool_free_node(reg_ctx->pos_cache_pool, pos_cache);
      pos_cache = deque_pop_front(ambi_ctx->center_queue, pos_cache_s, embed.deque_elem);
    }
  }

  avl_destruct(ambi_ctx->ambiguity_cache_eoso);
  avl_destruct(ambi_ctx->ambiguity_cache_soeo);

  afree(ambi_ctx);
}

void expr_activate_ambi_ctx(expr_ctx_t expr_ctx, reg_ctx_t context);

ambi_ctx_t ambi_ctx_alloc(expr_ambi_t expr_ambi) {
  ambi_ctx_t ambi_ctx = amalloc(sizeof(ambi_ctx_s));
  expr_ctx_init(&ambi_ctx->header, &expr_ambi->header, ambi_ctx_free, expr_activate_ambi_ctx);
  ambi_ctx->ambiguity_cache_eoso = avl_construct(pos_cache_cmp_eoso);
  ambi_ctx->ambiguity_cache_soeo = avl_construct(pos_cache_cmp_soeo);
  deque_init(ambi_ctx->center_queue);
  return ambi_ctx;
}

void expr_init_ambi(expr_ambi_t self, expr_t target, expr_feed_f feed) {
  expr_init(&self->header, target, feed);
}

void expr_feed_ambi_ambiguity(expr_t expr, pos_cache_t ambiguity, reg_ctx_t context) {
  expr_ambi_t self = container_of(expr, expr_ambi_s, header);

  ambi_ctx_t ambi_ctx;
  avl_node_t node = avl_search(context->expr_ctx_map, expr);
  if (node == NULL) {
    ambi_ctx = ambi_ctx_alloc(self);
    avl_insert(context->expr_ctx_map, expr, &ambi_ctx->header.avl_elem);
  } else {
    ambi_ctx = container_of(node, ambi_ctx_s, header.avl_elem);
  }

  pos_cache_t ambiguity2 = dynapool_alloc_node(context->pos_cache_pool);
  ambiguity2->pos = ambiguity->pos;
  avl_insert(ambi_ctx->ambiguity_cache_eoso, &ambiguity->pos, &ambiguity->embed.avl_elem);
  avl_insert(ambi_ctx->ambiguity_cache_soeo, &ambiguity2->pos, &ambiguity2->embed.avl_elem);
}

void expr_feed_ambi_center(expr_t expr, pos_cache_t center, reg_ctx_t context) {
  expr_ambi_t self = container_of(expr, expr_ambi_s, header);

  ambi_ctx_t ambi_ctx;
  avl_node_t node = avl_search(context->expr_ctx_map, expr);
  if (node == NULL) {
    ambi_ctx = ambi_ctx_alloc(self);
    avl_insert(context->expr_ctx_map, expr, &ambi_ctx->header.avl_elem);
  } else {
    ambi_ctx = container_of(node, ambi_ctx_s, header.avl_elem);
  }

  if (deque_empty(ambi_ctx->center_queue)) {
    prique_push(context->activate_queue, &ambi_ctx->header);
  }
  deque_push_back(ambi_ctx->center_queue, center, pos_cache_s, embed.deque_elem);
}

void expr_activate_ambi_ctx(expr_ctx_t expr_ctx, reg_ctx_t context) {
  ambi_ctx_t ambi_ctx = container_of(expr_ctx, ambi_ctx_s, header);
  pos_cache_t center = deque_pop_front(ambi_ctx->center_queue, pos_cache_s, embed.deque_elem);
  while (center != NULL) {
    if (avl_search_ext(ambi_ctx->ambiguity_cache_eoso, center, pos_cache_eo_in_word) == NULL &&
        avl_search_ext(ambi_ctx->ambiguity_cache_soeo, center, pos_cache_so_in_word) == NULL) {
      expr_feed_target(ambi_ctx->header.expr, center, context);
    } else {
      dynapool_free_node(context->pos_cache_pool, center);
    }
    center = deque_pop_front(ambi_ctx->center_queue, pos_cache_s, embed.deque_elem);
  }
}
