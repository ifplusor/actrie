/**
 * dist.c
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#include "dist.h"

#include <alib/collections/map/avl.h>

typedef struct _expression_distance_context_ {
  expr_ctx_s header;
  avl_t prefix_cache;
  avl_t suffix_cache;
} dist_ctx_s, *dist_ctx_t;

void dist_ctx_free(expr_ctx_t expr_ctx, reg_ctx_t reg_ctx) {
  dist_ctx_t dist_ctx = container_of(expr_ctx, dist_ctx_s, header);

  avl_walk_in_order(dist_ctx->prefix_cache, NULL, free_pos_cache, NULL, reg_ctx);
  avl_destruct(dist_ctx->prefix_cache);

  avl_walk_in_order(dist_ctx->suffix_cache, NULL, free_pos_cache, NULL, reg_ctx);
  avl_destruct(dist_ctx->suffix_cache);

  afree(dist_ctx);
}

dist_ctx_t dist_ctx_alloc(expr_dist_t expr_dist) {
  dist_ctx_t dist_ctx = amalloc(sizeof(dist_ctx_s));
  expr_ctx_init(&dist_ctx->header, &expr_dist->header, dist_ctx_free);
  dist_ctx->prefix_cache = avl_construct(pos_cache_cmp_eoso);
  dist_ctx->suffix_cache = avl_construct(pos_cache_cmp_soeo);
  return dist_ctx;
}

void expr_init_dist(expr_dist_t self, expr_t target, expr_feed_f feed, uint32_t min, uint32_t max) {
  expr_init(&self->header, target, feed);
  self->min = min;
  self->max = max;
}

static void prefix_match_suffix(avl_node_t node, void* arg) {
  pos_cache_t suffix = container_of(node, pos_cache_s, embed.avl_elem);
  expr_feed_arg_t feed_arg = (expr_feed_arg_t)arg;

  expr_t expr = feed_arg->expr;
  pos_cache_t prefix = feed_arg->keyword;
  reg_ctx_t reg_ctx = feed_arg->context;

  pos_cache_t keyword = dynapool_alloc_node(reg_ctx->pos_cache_pool);
  keyword->pos.so = prefix->pos.so;
  keyword->pos.eo = suffix->pos.eo;
  expr_feed_target(expr, keyword, reg_ctx);
}

void expr_feed_dist_prefix(expr_t expr, pos_cache_t prefix, reg_ctx_t context) {
  expr_dist_t self = container_of(expr, expr_dist_s, header);

  dist_ctx_t dist_ctx;
  avl_node_t node = avl_search(context->expr_ctx_map, expr);
  if (node == NULL) {
    dist_ctx = dist_ctx_alloc(self);
    avl_insert(context->expr_ctx_map, expr, &dist_ctx->header.avl_elem);
  } else {
    dist_ctx = container_of(node, dist_ctx_s, header.avl_elem);
  }

  avl_insert(dist_ctx->prefix_cache, &prefix->pos, &prefix->embed.avl_elem);

  strpos_s suffix_range = {.so = context->fix_pos_func(prefix->pos.eo, self->min, true, context->fix_pos_arg),
                           .eo = context->fix_pos_func(prefix->pos.eo, self->max, true, context->fix_pos_arg)};
  expr_feed_arg_s feed_arg = {.expr = expr, .keyword = prefix, .context = context};
  avl_walk_in_order(dist_ctx->suffix_cache, pos_cache_so_in_range, prefix_match_suffix, &suffix_range, &feed_arg);
}

static void suffix_match_prefix(avl_node_t node, void* arg) {
  pos_cache_t prefix = container_of(node, pos_cache_s, embed.avl_elem);
  expr_feed_arg_t feed_arg = (expr_feed_arg_t)arg;

  expr_t expr = feed_arg->expr;
  pos_cache_t suffix = feed_arg->keyword;
  reg_ctx_t reg_ctx = feed_arg->context;

  pos_cache_t keyword = dynapool_alloc_node(reg_ctx->pos_cache_pool);
  keyword->pos.so = prefix->pos.so;
  keyword->pos.eo = suffix->pos.eo;
  expr_feed_target(expr, keyword, reg_ctx);
}

void expr_feed_dist_suffix(expr_t expr, pos_cache_t suffix, reg_ctx_t context) {
  expr_dist_t self = container_of(expr, expr_dist_s, header);

  dist_ctx_t dist_ctx;
  avl_node_t node = avl_search(context->expr_ctx_map, expr);
  if (node == NULL) {
    dist_ctx = dist_ctx_alloc(self);
    avl_insert(context->expr_ctx_map, expr, &dist_ctx->header.avl_elem);
  } else {
    dist_ctx = container_of(node, dist_ctx_s, header.avl_elem);
  }

  avl_insert(dist_ctx->suffix_cache, &suffix->pos, &suffix->embed.avl_elem);

  strpos_s prefix_range = {.so = context->fix_pos_func(suffix->pos.so, self->max, false, context->fix_pos_arg),
                           .eo = context->fix_pos_func(suffix->pos.so, self->min, false, context->fix_pos_arg)};
  expr_feed_arg_s feed_arg = {.expr = expr, .keyword = suffix, .context = context};
  avl_walk_in_order(dist_ctx->prefix_cache, pos_cache_eo_in_range, suffix_match_prefix, &prefix_range, &feed_arg);
}
