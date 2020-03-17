/**
 * engine.c
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#include "engine.h"

#include "expr/expr.h"

extern inline void expr_init(expr_t self, expr_t target, expr_feed_f feed);
extern inline void expr_feed_target(expr_t self, pos_cache_t keyword, reg_ctx_t context);

extern inline void expr_ctx_init(expr_ctx_t self, expr_t expr, expr_ctx_free_f free, expr_ctx_activate_f activate);

//
// compare

sptr_t pos_cache_cmp_output(void* node1, void* node2) {
  pos_cache_t pos1 = node1, pos2 = node2;
  if (pos1->pos.eo == pos2->pos.eo) {
    return pos1->pos.so - pos2->pos.so;
  } else {
    return pos1->pos.eo - pos2->pos.eo;
  }
}

sptr_t pos_cache_cmp_eoso(avl_node_t node, void* key) {
  pos_cache_t pos_cache = container_of(node, pos_cache_s, embed.avl_elem);
  strpos_t pos_key = (strpos_t)key;
  if (pos_cache->pos.eo == pos_key->eo) {
    return pos_cache->pos.so - pos_key->so;
  } else {
    return pos_cache->pos.eo - pos_key->eo;
  }
}

sptr_t pos_cache_cmp_soeo(avl_node_t node, void* key) {
  pos_cache_t pos_cache = container_of(node, pos_cache_s, embed.avl_elem);
  strpos_t pos_key = (strpos_t)key;
  if (pos_cache->pos.so == pos_key->so) {
    return pos_cache->pos.eo - pos_key->eo;
  } else {
    return pos_cache->pos.so - pos_key->so;
  }
}

sptr_t pos_cache_eq_eo(avl_node_t node, void* key) {
  pos_cache_t pos_cache = container_of(node, pos_cache_s, embed.avl_elem);
  size_t* pos_eo = (size_t*)key;
  return pos_cache->pos.eo - *pos_eo;
}

//
// in range

sptr_t pos_cache_so_in_range(avl_node_t node, void* arg) {
  pos_cache_t pos_cache = container_of(node, pos_cache_s, embed.avl_elem);
  strpos_t pos_range = (strpos_t)arg;
  if (pos_cache->pos.so < pos_range->so) {
    return -1;
  } else if (pos_cache->pos.so > pos_range->eo) {
    return 1;
  } else {
    return 0;
  }
}

sptr_t pos_cache_eo_in_range(avl_node_t node, void* arg) {
  pos_cache_t pos_cache = container_of(node, pos_cache_s, embed.avl_elem);
  strpos_t pos_range = (strpos_t)arg;
  if (pos_cache->pos.eo < pos_range->so) {
    return -1;
  } else if (pos_cache->pos.eo > pos_range->eo) {
    return 1;
  } else {
    return 0;
  }
}

sptr_t pos_cache_so_in_word(avl_node_t node, void* arg) {
  pos_cache_t pos_cache = container_of(node, pos_cache_s, embed.avl_elem);
  strpos_t pos_word = (strpos_t)arg;
  if (pos_cache->pos.so >= pos_word->eo) {
    return 1;
  } else if (pos_cache->pos.so < pos_word->so && pos_cache->pos.eo <= pos_word->so) {
    return -1;
  } else {
    return 0;
  }
}

sptr_t pos_cache_eo_in_word(avl_node_t node, void* arg) {
  pos_cache_t pos_cache = container_of(node, pos_cache_s, embed.avl_elem);
  strpos_t pos_word = (strpos_t)arg;
  if (pos_cache->pos.eo <= pos_word->so) {
    return -1;
  } else if (pos_cache->pos.eo > pos_word->eo && pos_cache->pos.so >= pos_word->eo) {
    return 1;
  } else {
    return 0;
  }
}

// free for avl

void free_pos_cache(avl_node_t node, void* arg) {
  pos_cache_t cache_node = container_of(node, pos_cache_s, embed.avl_elem);
  reg_ctx_t reg_ctx = (reg_ctx_t)arg;
  dynapool_free_node(reg_ctx->pos_cache_pool, cache_node);
}

//
// reglet

reglet_t reglet_alloc() {
  reglet_t reglet = amalloc(sizeof(reglet_s));
  reglet->expr_pool = NULL;
  reglet->trie = NULL;
  return reglet;
}

void reglet_free(reglet_t reglet) {
  afree(reglet);
}

reglet_t reglet_construct() {
  reglet_t reglet = reglet_alloc();
  size_t expr_size = sizeof(expr_s);
  expr_size = alib_max(expr_size, sizeof(expr_text_s));
  expr_size = alib_max(expr_size, sizeof(expr_dist_s));
  expr_size = alib_max(expr_size, sizeof(expr_ambi_s));
  expr_size = alib_max(expr_size, sizeof(expr_anto_s));
  expr_size = alib_max(expr_size, sizeof(expr_pass_s));
  reglet->expr_pool = dynapool_construct(expr_size);
  reglet->extra_store = trie_alloc();
  reglet->trie = trie_alloc();
  return reglet;
}

static void free_extra_in_store(trie_t trie, void* node) {
  dstr_t ds = (dstr_t)node;
  _release(ds);
}

void reglet_destruct(reglet_t reglet) {
  dynapool_destruct(reglet->expr_pool);
  trie_free(reglet->extra_store, free_extra_in_store);
  trie_free(reglet->trie, NULL);
  reglet_free(reglet);
}

static expr_t reglet_build_expr(reglet_t self, ptrn_t pattern, expr_t target, expr_feed_f feed);

static expr_t reglet_build_expr_for_pure(reglet_t self, ptrn_t pattern, expr_t target, expr_feed_f feed) {
  dstr_t text = pattern->desc;
  expr_text_t expr_text = dynapool_alloc_node(self->expr_pool);
  expr_init_text(expr_text, target, feed, text->len);
  // TODO: replace list_t
  list_t expr_list = trie_search(self->trie, text->str, text->len);
  expr_list = _(list, &expr_text->header, cons, expr_list);
  expr_list = trie_add_keyword(self->trie, text->str, text->len, expr_list);
  _release(expr_list);
  return &expr_text->header;
}

static expr_t reglet_build_expr_for_ambi(reglet_t self, ptrn_t pattern, expr_t target, expr_feed_f feed) {
  list_t con = pattern->desc;
  expr_ambi_t expr_ambi = dynapool_alloc_node(self->expr_pool);
  expr_init_ambi(expr_ambi, target, feed);
  ptrn_t center = _(list, con, car);
  ptrn_t ambiguity = _(list, con, cdr);
  reglet_build_expr(self, center, &expr_ambi->header, expr_feed_ambi_center);
  reglet_build_expr(self, ambiguity, &expr_ambi->header, expr_feed_ambi_ambiguity);
  return &expr_ambi->header;
}

static expr_t reglet_build_expr_for_anto(reglet_t self, ptrn_t pattern, expr_t target, expr_feed_f feed) {
  list_t con = pattern->desc;
  expr_anto_t expr_anto = dynapool_alloc_node(self->expr_pool);
  expr_init_anto(expr_anto, target, feed);
  ptrn_t center = _(list, con, car);
  ptrn_t antonym = _(list, con, cdr);
  reglet_build_expr(self, center, &expr_anto->header, expr_feed_anto_center);
  reglet_build_expr(self, antonym, &expr_anto->header, expr_feed_anto_antonym);
  return &expr_anto->header;
}

static expr_t reglet_build_expr_for_dist(reglet_t self, ptrn_t pattern, expr_t target, expr_feed_f feed) {
  pdd_t pdd = pattern->desc;
  expr_dist_t expr_dist = dynapool_alloc_node(self->expr_pool);
  expr_init_dist(expr_dist, target, feed, pdd->min, pdd->max);
  if (pdd->type == ptrn_dist_type_num) {
    reglet_build_expr(self, pdd->head, &expr_dist->header, expr_feed_ddist_prefix);
    reglet_build_expr(self, pdd->tail, &expr_dist->header, expr_feed_ddist_suffix);
  } else {
    reglet_build_expr(self, pdd->head, &expr_dist->header, expr_feed_dist_prefix);
    reglet_build_expr(self, pdd->tail, &expr_dist->header, expr_feed_dist_suffix);
  }
  return &expr_dist->header;
}

static expr_t reglet_build_expr_for_alter(reglet_t self, ptrn_t pattern, expr_t target, expr_feed_f feed) {
  expr_pass_t expr_pass = dynapool_alloc_node(self->expr_pool);
  expr_init_pass(expr_pass, target, feed);
  for (list_t con = pattern->desc; con != NULL; con = con->cdr) {
    ptrn_t sub_ptrn = con->car;
    reglet_build_expr(self, sub_ptrn, &expr_pass->header, expr_feed_pass);
  }
  return &expr_pass->header;
}

static expr_t reglet_build_expr(reglet_t self, ptrn_t pattern, expr_t target, expr_feed_f feed) {
  switch (pattern->type) {
    case ptrn_type_pure:
      return reglet_build_expr_for_pure(self, pattern, target, feed);
    case ptrn_type_anti_ambi:
      return reglet_build_expr_for_ambi(self, pattern, target, feed);
    case ptrn_type_anti_anto:
      return reglet_build_expr_for_anto(self, pattern, target, feed);
    case ptrn_type_dist:
      return reglet_build_expr_for_dist(self, pattern, target, feed);
    case ptrn_type_alter:
      return reglet_build_expr_for_alter(self, pattern, target, feed);
    default:
      break;
  }
  return NULL;
}

typedef struct _regex_exprerssion_output_ {
  expr_s header;
  dstr_t extra;
} expr_output_s, *expr_output_t;

static void expr_init_output(expr_output_t self, dstr_t extra) {
  expr_init(&self->header, NULL, NULL);
  self->extra = extra;
}

static void expr_feed_output(expr_t expr, pos_cache_t keyword, reg_ctx_t context) {
  expr_output_t self = container_of(expr, expr_output_s, header);
  keyword->embed.extra = self->extra;
  // push into output queue
  prique_push(context->output_queue, keyword);
}

void reglet_add_pattern(reglet_t self, ptrn_t pattern, strlen_t extra) {
  expr_output_t expr_output = dynapool_alloc_node(self->expr_pool);
  dstr_t ds = NULL;
  if (extra->len > 0) {
    ds = trie_search(self->extra_store, extra->ptr, extra->len);
    if (ds == NULL) {
      ds = dstr(extra);
      trie_add_keyword(self->extra_store, extra->ptr, extra->len, ds);
    }
  }
  expr_init_output(expr_output, ds);
  reglet_build_expr(self, pattern, &expr_output->header, expr_feed_output);
}

sptr_t expr_ctx_cmp(avl_node_t node, void* key) {
  expr_ctx_t expr_ctx = container_of(node, expr_ctx_s, avl_elem);
  return expr_ctx->expr - (expr_t)key;
}

sptr_t expr_ctx_cmp2(void* node1, void* node2) {
  expr_ctx_t expr_ctx1 = node1, expr_ctx2 = node2;
  return -(expr_ctx1->expr - expr_ctx2->expr);
}

static size_t default_fix_pos(size_t pos, size_t diff, bool plus_or_subtract, void* arg) {
  if (plus_or_subtract) {
    return pos + diff;
  } else {
    if (diff >= pos) {
      return 0;
    } else {
      return pos - diff;
    }
  }
}

reg_ctx_t reglet_alloc_context(reglet_t reglet) {
  reg_ctx_t reg_ctx = amalloc(sizeof(reg_ctx_s));
  reg_ctx->pos_cache_pool = dynapool_construct_with_type(pos_cache_s);
  reg_ctx->expr_ctx_map = avl_construct(expr_ctx_cmp);
  reg_ctx->output_queue = prique_construct(pos_cache_cmp_output);
  reg_ctx->activate_queue = prique_construct(expr_ctx_cmp2);
  reg_ctx->fix_pos_func = default_fix_pos;
  reg_ctx->fix_pos_arg = NULL;
  return reg_ctx;
}

void free_expr_ctx(avl_node_t node, void* arg) {
  expr_ctx_t expr_ctx = container_of(node, expr_ctx_s, avl_elem);
  reg_ctx_t reg_ctx = (reg_ctx_t)arg;
  expr_ctx->free_func(expr_ctx, reg_ctx);
}

void reglet_free_context(reg_ctx_t context) {
  if (context != NULL) {
    context->reset_or_free = false;
    // free expr_ctx and map
    avl_walk_in_order(context->expr_ctx_map, NULL, free_expr_ctx, NULL, context);
    avl_destruct(context->expr_ctx_map);
    // free pos_cache pool
    dynapool_destruct(context->pos_cache_pool);
    // free output queue
    prique_destruct(context->output_queue);
    // free activate queue
    prique_destruct(context->activate_queue);
    // free context
    afree(context);
  }
}

void reglet_reset_context(reg_ctx_t context, char content[], size_t len) {
  if (context != NULL) {
    context->content = (strlen_s){.ptr = content, .len = len};

    context->reset_or_free = true;
    // free expression context, and clear map
    avl_walk_in_order(context->expr_ctx_map, NULL, free_expr_ctx, NULL, context);
    avl_reset(context->expr_ctx_map);
    // clear output_queue
    for (size_t i = 1; i <= context->output_queue->len; i++) {
      dynapool_free_node(context->pos_cache_pool, context->output_queue->data);
    }
    context->output_queue->len = 0;
    // clear activate expr_ctx queue
    context->activate_queue->len = 0;
  }
}

void reglet_fix_pos(reg_ctx_t context, fix_pos_f fix_pos_func, void* fix_pos_arg) {
  if (fix_pos_func != NULL) {
    context->fix_pos_func = fix_pos_func;
    context->fix_pos_arg = fix_pos_arg;
  } else {
    context->fix_pos_func = default_fix_pos;
    context->fix_pos_arg = NULL;
  }
}

void reglet_activate_expr_ctx(reg_ctx_t context) {
  expr_ctx_t expr_ctx = prique_pop(context->activate_queue);
  while (expr_ctx != NULL) {
    expr_ctx->activate_func(expr_ctx, context);
    expr_ctx = prique_pop(context->activate_queue);
  }
}
