//
// Created by james on 6/19/17.
//

#include <regex.h>
#include "utf8.h"
#include "distance.h"
#include "acdat.h"

/* 内部接口 */
extern trie_ptr trie_alloc();

extern bool trie_add_keyword(trie_ptr self,
                             const unsigned char keyword[],
                             size_t len,
                             match_dict_index_ptr index);

extern void trie_sort_to_line(trie_ptr self);

extern dat_trie_ptr dat_construct(trie_ptr origin, bool enable_automation);


// Distance Matcher
// ========================================================


const matcher_func dist_matcher_func = {
    .destruct = (matcher_destruct_func) dist_destruct,
    .alloc_context = (matcher_alloc_context_func) dist_alloc_context
};

const context_func dist_context_func = {
    .free_context = (matcher_free_context_func) dist_free_context,
    .reset_context = (matcher_reset_context_func) dist_reset_context,
    .next = (matcher_next_func) dist_next_on_index
};

static const char
    *pattern = "(.*)(\\.\\*\\?|\\?\\(([0-9]|1[0-5])\\)\\+{5})(.*)";
static regex_t reg;
static bool pattern_compiled = false;

void dict_distance_before_reset(match_dict_ptr dict,
                                size_t *index_count,
                                size_t *buffer_size)
{
  *index_count *= 2;
  *buffer_size *= 2;
}

bool dict_distance_add_keyword_and_extra(match_dict_ptr dict,
                                         char keyword[],
                                         char extra[])
{
  size_t key_cur, head_cur, tail_cur;
  size_t tag, distance;
  regmatch_t pmatch[5];
  char dist[3], *split;
  int err;

  if (!pattern_compiled) {
    // compile pattern
    err = regcomp(&reg, pattern, REG_EXTENDED | REG_NEWLINE);
    if (err) {
      fprintf(stderr, "error: compile regex failed!\n");
      return false;
    } else {
      pattern_compiled = true;
    }
  }

  if (keyword != NULL && keyword[0] != '\0') {
    err = regexec(&reg, keyword, 5, pmatch, 0);
    if (err == REG_NOMATCH) {
      return true;
    } else if (err != REG_NOERROR) {
      return false;
    }

    if (pmatch[3].rm_so == -1) {
      // .*?
      distance = 15;
    } else {
      // +++++
      // 连用需要多条 index
      if (pmatch[3].rm_so == pmatch[3].rm_eo) {
        dist[0] = keyword[pmatch[3].rm_so];
        dist[1] = '\0';
      } else {
        dist[0] = keyword[pmatch[3].rm_so];
        dist[1] = keyword[pmatch[3].rm_eo];
        dist[2] = '\0';
      }
      distance = (size_t) atoi(dist);
    }

    strcpy(dict->buffer + dict->_cursor, keyword);
    key_cur = dict->_cursor;
    dict->_cursor += strlen(keyword) + 1;

    strncpy(dict->buffer + dict->_cursor, &keyword[pmatch[1].rm_so],
            (size_t) (pmatch[1].rm_eo - pmatch[1].rm_so));
    head_cur = dict->_cursor;
    dict->_cursor += (size_t) (pmatch[1].rm_eo - pmatch[1].rm_so) + 1;

    strncpy(dict->buffer + dict->_cursor, &keyword[pmatch[4].rm_so],
            (size_t) (pmatch[4].rm_eo - pmatch[4].rm_so));
    tail_cur = dict->_cursor;
    dict->_cursor += (size_t) (pmatch[4].rm_eo - pmatch[4].rm_so) + 1;

    tag = dict->idx_count;

    // 针对连用，需要扩充内存
    for (split = strtok(dict->buffer + head_cur, ","); split;
         split = strtok(NULL, ",")) {
      size_t length = strlen(split);
      if (length > dict->max_key_length) dict->max_key_length = length;
      dict_add_index(dict, strlen(split), split, dict->buffer + key_cur,
                     (char *) tag, match_dict_keyword_type_head);
    }

    for (split = strtok(dict->buffer + tail_cur, ","); split;
         split = strtok(NULL, ",")) {
      size_t length = strlen(split);
      if (length > dict->max_extra_length) dict->max_extra_length = length;
      dict_add_index(dict, strlen(split), split, (char *) distance,
                     (char *) tag, match_dict_keyword_type_tail);
    }
  }

  return true;
}

bool dist_destruct(dist_matcher_t p)
{
  if (p != NULL) {
    if (p->_head_matcher != NULL) dat_destruct((dat_trie_ptr) p->_head_matcher);
    if (p->_tail_matcher != NULL) dat_destruct((dat_trie_ptr) p->_tail_matcher);
    if (p->_dict != NULL) dict_release(p->_dict);
    free(p);
    return true;
  }
  return false;
}

trie_ptr dist_trie_filter_construct(match_dict_ptr dict,
                                    match_dict_keyword_type filter)
{
  trie_ptr prime_trie = trie_alloc();
  if (prime_trie == NULL) return NULL;

  prime_trie->_dict = dict_assign(dict);

  for (size_t i = 0; i < dict->idx_count; i++) {
    match_dict_index_ptr index = &dict->index[i];
    if (index->type != filter) continue;

    if (!trie_add_keyword(prime_trie,
                          (const unsigned char *) index->keyword,
                          index->length,
                          index)) {
      fprintf(stderr, "fatal: encounter error when add keywords.\n");
      trie_destruct(prime_trie);
      prime_trie = NULL;
      break;
    }
  }

  if (prime_trie != NULL) {
    trie_sort_to_line(prime_trie);  /* 排序字典树节点 for bfs and binary-search */
  }

  return prime_trie;
}

dist_matcher_t dist_construct(match_dict_ptr dict, bool enable_automation)
{
  trie_ptr head_trie, tail_trie;
  dist_matcher_t matcher;

  head_trie = dist_trie_filter_construct(dict, match_dict_keyword_type_head);
  if (head_trie == NULL) return NULL;

  tail_trie = dist_trie_filter_construct(dict, match_dict_keyword_type_tail);
  if (tail_trie == NULL) return NULL;

  matcher = malloc(sizeof(struct dist_matcher));
  if (matcher == NULL) return NULL;

  matcher->_dict = dict_assign(dict);

  matcher->_head_matcher =
      (matcher_t) dat_construct(head_trie, enable_automation);
  matcher->_head_matcher->_func = dat_matcher_func;
  matcher->_head_matcher->_type =
      enable_automation ? matcher_type_acdat : matcher_type_dat;
  trie_destruct(head_trie);

  matcher->_tail_matcher =
      (matcher_t) dat_construct(tail_trie, enable_automation);
  matcher->_tail_matcher->_func = dat_matcher_func;
  matcher->_tail_matcher->_type =
      enable_automation ? matcher_type_acdat : matcher_type_dat;
  trie_destruct(tail_trie);

  return matcher;
}

dist_matcher_t dist_construct_by_file(const char *path, bool enable_automation)
{
  dist_matcher_t dist_matcher = NULL;
  match_dict_ptr dict = NULL;
  FILE *fp = NULL;

  if (path == NULL) {
    return NULL;
  }

  fp = fopen(path, "rb");
  if (fp == NULL) return NULL;

  dict = dict_alloc();
  if (dict == NULL) return NULL;
  dict->add_keyword_and_extra = dict_distance_add_keyword_and_extra;
  dict->before_reset = dict_distance_before_reset;

  if (dict_parser_by_file(dict, fp)) {
    dist_matcher = dist_construct(dict, enable_automation);
  }

  fclose(fp);
  dict_release(dict);

  fprintf(stderr,
          "construct trie %s!\n",
          dist_matcher != NULL ? "success" : "failed");
  return dist_matcher;
}

dist_matcher_t dist_construct_by_string(const char *string,
                                        bool enable_automation)
{
  dist_matcher_t dist_matcher = NULL;
  match_dict_ptr dict = NULL;

  if (string == NULL) {
    return NULL;
  }

  dict = dict_alloc();
  if (dict == NULL) return NULL;
  dict->add_keyword_and_extra = dict_distance_add_keyword_and_extra;
  dict->before_reset = dict_distance_before_reset;

  if (dict_parser_by_s(dict, string)) {
    dist_matcher = dist_construct(dict, enable_automation);
  }

  dict_release(dict);

  fprintf(stderr,
          "construct trie %s!\n",
          dist_matcher != NULL ? "success" : "failed");
  return dist_matcher;
}

dist_context_t dist_alloc_context(dist_matcher_t matcher)
{
  dist_context_t ctx = malloc(sizeof(struct dist_context));
  if (ctx == NULL) return NULL;

  ctx->_matcher = matcher;
  ctx->_head_context = matcher_alloc_context(matcher->_head_matcher);
  if (ctx->_head_context == NULL) goto dist_alloc_context_failed;
  ctx->_tail_context = matcher_alloc_context(matcher->_tail_matcher);
  if (ctx->_tail_context == NULL) goto dist_alloc_context_failed;
  ctx->header.out_matched_index = &ctx->out_index;
  ctx->_utf8_pos = NULL;
  return ctx;

dist_alloc_context_failed:
  if (ctx->_tail_context != NULL) matcher_free_context(ctx->_tail_context);
  if (ctx->_head_context != NULL) matcher_free_context(ctx->_head_context);
  free(ctx);
  return NULL;
}

bool dist_free_context(dist_context_t ctx)
{
  if (ctx != NULL) {
    if (ctx->_utf8_pos != NULL) free(ctx->_utf8_pos);
    if (ctx->_tail_context != NULL) matcher_free_context(ctx->_tail_context);
    if (ctx->_head_context != NULL) matcher_free_context(ctx->_head_context);
    free(ctx);
  }
  return true;
}

bool dist_reset_context(dist_context_t context, unsigned char content[],
                        size_t len)
{
  if (context == NULL) return false;

  context->header.content = content;
  context->header.len = len;
  context->header.out_matched_index = &context->out_index;
  context->header.out_e = 0;
  context->_c = content[0];

  if (context->_utf8_pos != NULL) {
    free(context->_utf8_pos);
    context->_utf8_pos = NULL;
  }
  context->_utf8_pos = malloc((len + 1) * sizeof(size_t));
  if (context->_utf8_pos == NULL) {
    fprintf(stderr, "error when malloc.\n");
  }
  utf8_word_position((const char *) content, len, context->_utf8_pos);

  matcher_reset_context(context->_head_context, (char *) content, len);
  matcher_reset_context(context->_tail_context, (char *) content, len);

  context->_hcnt = 0;
  context->_htidx = 0;
  context->_state = dist_match_state_new_round;

  return false;
}

bool dist_construct_out(dist_context_t ctx, size_t _e)
{
  // alias
  unsigned char *content = ctx->header.content;
  context_t hctx = ctx->_head_context;

  ctx->header.out_e = _e;
  ctx->out_index.length = ctx->header.out_e - hctx->out_e
      + hctx->out_matched_index->length;
  ctx->out_index.wlen =
      utf8_word_distance(ctx->_utf8_pos,
                         hctx->out_e - hctx->out_matched_index->length,
                         ctx->header.out_e);
  ctx->_c = content[ctx->header.out_e];
  content[ctx->header.out_e] = '\0';
  ctx->out_index.keyword = (const char *)
      &content[hctx->out_e - hctx->out_matched_index->length];
  ctx->out_index.extra = hctx->out_matched_index->extra;

  return true;
}

bool dist_next_on_index(dist_context_t ctx)
{
  // alias
  unsigned char *content = ctx->header.content;
  context_t hist = ctx->_history_context;
  context_t hctx = ctx->_head_context;
  context_t tctx = ctx->_tail_context;

  content[ctx->header.out_e] = ctx->_c;  // recover content

  switch (ctx->_state) {
    case dist_match_state_new_round: break;
    case dist_match_state_check_history: goto check_history;
    case dist_match_state_check_tail: goto check_tail;
  }

  while (dat_ac_next_on_index((dat_context_ptr) hctx)) {
    ctx->_state = dist_match_state_check_history;
    for (ctx->_i = (HISTORY_SIZE + ctx->_htidx - ctx->_hcnt) % HISTORY_SIZE;
         ctx->_i != ctx->_htidx; ctx->_i = (ctx->_i + 1) % HISTORY_SIZE) {
      if (hist[ctx->_i].out_e > hctx->out_e) break;
      ctx->_hcnt--;
    }
    ctx->_i--;
check_history:
    for (ctx->_i = (ctx->_i + 1) % HISTORY_SIZE; ctx->_i != ctx->_htidx;
         ctx->_i = (ctx->_i + 1) % HISTORY_SIZE) {
      long diff_pos =
          utf8_word_distance(ctx->_utf8_pos, hctx->out_e, hist[ctx->_i].out_e);
      long distance = diff_pos - hist[ctx->_i].out_matched_index->wlen;
      if (distance > 15) {  // max distance is 15
        if (hist[ctx->_i].out_e - hctx->out_e >
            45 + ctx->_matcher->_dict->max_extra_length)
          goto next_round;
        continue;
      }

      match_dict_index_ptr matched_index = hist[ctx->_i].out_matched_index;
      for (; matched_index != NULL; matched_index = matched_index->next) {
        if (matched_index->tag <= hctx->out_matched_index->tag) break;
      }
      if (matched_index != NULL
          && matched_index->tag == hctx->out_matched_index->tag) {
        if (distance <= (size_t) matched_index->extra)
          return dist_construct_out(ctx, hist[ctx->_i].out_e);
        break;
      }
    }

    ctx->_state = dist_match_state_check_tail;
check_tail:
    // one node will match the tag only once.
    while (dat_ac_next_on_node((dat_context_ptr) tctx)) {
      long diff_pos =
          utf8_word_distance(ctx->_utf8_pos, hctx->out_e, tctx->out_e);
      long distance = (long) (diff_pos - tctx->out_matched_index->wlen);
      if (distance < 0) continue;

      // record history
      hist[ctx->_htidx] = *tctx;
      ctx->_htidx = (ctx->_htidx + 1) % HISTORY_SIZE;
      ctx->_hcnt++;

      if (distance > 15) {  // max distance is 15
        if (tctx->out_e - hctx->out_e >
            45 + ctx->_matcher->_dict->max_extra_length)
          goto next_round;
        continue;
      }

      match_dict_index_ptr matched_index = tctx->out_matched_index;
      for (; matched_index != NULL; matched_index = matched_index->next) {
        // link table's tag is descending order
        if (matched_index->tag <= hctx->out_matched_index->tag) break;
      }
      if (matched_index != NULL
          && matched_index->tag == hctx->out_matched_index->tag) {
        if (distance <= (size_t) matched_index->extra)
          return dist_construct_out(ctx, tctx->out_e);
        break;
      }
    }
next_round:;
    ctx->_state = dist_match_state_new_round;
  }

  return false;
}
