//
// Created by james on 6/19/17.
//

#include <regex.h>
#include "utf8.h"
#include "distance.h"
#include "acdat.h"

#define MAX_WORD_DISTANCE 15
#define MAX_CHAR_DISTANCE (MAX_WORD_DISTANCE*3)

#define DIGITAL_MASK 0x00000100


// Distance Matcher
// ========================================================


const matcher_func_l dist_matcher_func = {
    .destruct = (matcher_destruct_func) dist_destruct,
    .alloc_context = (matcher_alloc_context_func) dist_alloc_context
};

const context_func_l dist_context_func = {
    .free_context = (matcher_free_context_func) dist_free_context,
    .reset_context = (matcher_reset_context_func) dist_reset_context,
    .next = (matcher_next_func) dist_next_on_index
};

/*
 * pattern will match:
 * - A.*?B
 * - A\d{0,5}B
 * - A.{0,5}B
 */
static const char
    *pattern = "(.*)(\\.\\*\\?|(\\\\d|\\.)\\{0,([0-9]|1[0-5])\\})(.*)";
static regex_t reg;
static bool pattern_compiled = false;
static const char *tokens_delimiter = "|";

void dict_distance_before_reset(match_dict_t dict,
                                size_t *index_count,
                                size_t *buffer_size) {
  *index_count *= 2;
  *buffer_size *= 2;
}

bool dict_distance_add_keyword_and_extra(match_dict_t dict,
                                         char keyword[],
                                         char extra[]) {
  char *key_cur, *head_cur, *tail_cur;
  size_t tag, distance;
  regmatch_t pmatch[6];
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
    err = regexec(&reg, keyword, 6, pmatch, 0);
    if (err == REG_NOMATCH) {
      return true;
    } else if (err != REG_NOERROR) {
      return false;
    }

    if (pmatch[3].rm_so == -1) {
      // .*?
      distance = MAX_WORD_DISTANCE;
    } else {

      if (pmatch[4].rm_so + 1 == pmatch[4].rm_eo) {
        dist[0] = keyword[pmatch[4].rm_so];
        dist[1] = '\0';
      } else {
        dist[0] = keyword[pmatch[4].rm_so];
        dist[1] = keyword[pmatch[4].rm_so + 1];
        dist[2] = '\0';
      }
      distance = (size_t) atoi(dist);

      if (keyword[pmatch[3].rm_so] == '.') {
        // .{0,n}
        ;
      } else {
        // \d{0,n}
        distance |= DIGITAL_MASK;
      }
    }

    // store original keyword
    key_cur = dynabuf_write(&dict->buffer, keyword, strlen(keyword) + 1);

    // store processed keyword

    // 1. store head
    head_cur =
        dynabuf_write_with_zero(&dict->buffer, &keyword[pmatch[1].rm_so],
                                (size_t) (pmatch[1].rm_eo - pmatch[1].rm_so));

    // 2. store tail
    tail_cur =
        dynabuf_write_with_zero(&dict->buffer, &keyword[pmatch[5].rm_so],
                                (size_t) (pmatch[5].rm_eo - pmatch[5].rm_so));

    tag = dict->idx_count;

    // 连用需要多条 index
    // 针对连用，需要扩充内存
    for (split = strtok(head_cur, tokens_delimiter); split;
         split = strtok(NULL, tokens_delimiter)) {
      size_t length = strlen(split);
      if (length > dict->max_key_length) dict->max_key_length = length;
      dict_add_index(dict, strlen(split), split, (char *) distance,
                     (char *) tag, match_dict_index_prop_head);
    }

    for (split = strtok(tail_cur, tokens_delimiter); split;
         split = strtok(NULL, tokens_delimiter)) {
      size_t length = strlen(split);
      if (length > dict->max_extra_length) dict->max_extra_length = length;
      dict_add_index(dict, strlen(split), split, key_cur,
                     (char *) tag, match_dict_index_prop_tail);
    }
  }

  return true;
}

bool dist_destruct(dist_matcher_t self) {
  if (self != NULL) {
    if (self->_head_matcher != NULL) dat_destruct((datrie_t) self->_head_matcher);
    if (self->_tail_matcher != NULL) dat_destruct((datrie_t) self->_tail_matcher);
    if (self->_dict != NULL) dict_release(self->_dict);
    free(self);
    return true;
  }
  return false;
}

dist_matcher_t dist_construct_by_dict(match_dict_t dict,
                                      bool enable_automation) {
  trie_t head_trie, tail_trie;
  dist_matcher_t matcher;

  head_trie =
      trie_construct_by_dict(dict, match_dict_index_prop_head, false);
  if (head_trie == NULL) return NULL;

  tail_trie =
      trie_construct_by_dict(dict, match_dict_index_prop_tail, false);
  if (tail_trie == NULL) return NULL;

  matcher = malloc(sizeof(struct dist_matcher));
  if (matcher == NULL) return NULL;

  matcher->_dict = dict_retain(dict);

  matcher->_head_matcher = (matcher_t)
      dat_construct_by_trie(head_trie, enable_automation);
  matcher->_head_matcher->_func = dat_matcher_func;
  matcher->_head_matcher->_type =
      enable_automation ? matcher_type_acdat : matcher_type_dat;
  trie_destruct(head_trie);

  matcher->_tail_matcher = (matcher_t)
      dat_construct_by_trie(tail_trie, enable_automation);
  matcher->_tail_matcher->_func = dat_matcher_func;
  matcher->_tail_matcher->_type =
      enable_automation ? matcher_type_acdat : matcher_type_dat;
  trie_destruct(tail_trie);

  return matcher;
}

dist_matcher_t dist_construct(vocab_t vocab, bool enable_automation) {
  dist_matcher_t dist_matcher = NULL;
  match_dict_t dict = NULL;
  FILE *fp = NULL;

  if (vocab == NULL) {
    return NULL;
  }

  dict = dict_alloc();
  if (dict == NULL) return NULL;
  dict->add_keyword_and_extra = dict_distance_add_keyword_and_extra;
  dict->before_reset = dict_distance_before_reset;

  if (dict_parse(dict, vocab)) {
    dist_matcher = dist_construct_by_dict(dict, enable_automation);
  }

  dict_release(dict);

  fprintf(stderr,
          "construct trie %s!\n",
          dist_matcher != NULL ? "success" : "failed");
  return dist_matcher;
}

dist_context_t dist_alloc_context(dist_matcher_t matcher) {
  dist_context_t ctx = malloc(sizeof(struct dist_context));
  if (ctx == NULL) return NULL;

  ctx->_matcher = matcher;
  ctx->_head_context = matcher_alloc_context(matcher->_head_matcher);
  if (ctx->_head_context == NULL) goto dist_alloc_context_failed;
  ctx->_tail_context = matcher_alloc_context(matcher->_tail_matcher);
  if (ctx->_tail_context == NULL) goto dist_alloc_context_failed;
  ctx->_digit_context = matcher_alloc_context(matcher->_tail_matcher);
  if (ctx->_digit_context == NULL) goto dist_alloc_context_failed;
  ctx->header.out_matched_index = &ctx->out_index;
  ctx->_utf8_pos = NULL;
  return ctx;

dist_alloc_context_failed:
  if (ctx->_tail_context != NULL) matcher_free_context(ctx->_tail_context);
  if (ctx->_head_context != NULL) matcher_free_context(ctx->_head_context);
  if (ctx->_digit_context != NULL) matcher_free_context(ctx->_digit_context);
  free(ctx);
  return NULL;
}

bool dist_free_context(dist_context_t ctx) {
  if (ctx != NULL) {
    if (ctx->_utf8_pos != NULL) free(ctx->_utf8_pos);
    if (ctx->_tail_context != NULL) matcher_free_context(ctx->_tail_context);
    if (ctx->_head_context != NULL) matcher_free_context(ctx->_head_context);
    if (ctx->_digit_context != NULL) matcher_free_context(ctx->_digit_context);
    free(ctx);
  }
  return true;
}

bool dist_reset_context(dist_context_t context, unsigned char content[],
                        size_t len) {
  if (context == NULL) return false;

  context->header.content = content;
  context->header.len = len;
  context->header.out_matched_index = &context->out_index;
  context->header.out_e = 0;
#ifdef REPLACE_BY_ZERO
  context->_c = content[0];
#endif

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

bool dist_construct_out(dist_context_t ctx, const char *extra, size_t _e) {
  // alias
  unsigned char *content = ctx->header.content;
  context_t hctx = ctx->_head_context;
  context_t tctx = ctx->_tail_context;

  ctx->header.out_e = _e;
  ctx->out_index.length = ctx->header.out_e - hctx->out_e
      + hctx->out_matched_index->length;
#ifdef USE_SUBPATTERN_LENGTH
  // NOTE: here use sub-pattern word length.
  ctx->out_index.wlen = hctx->out_matched_index->wlen + tctx->out_matched_index->wlen;
#else
  ctx->out_index.wlen =
      utf8_word_distance(ctx->_utf8_pos,
                         hctx->out_e - hctx->out_matched_index->length,
                         ctx->header.out_e);
#endif
#ifdef REPLACE_BY_ZERO
  ctx->_c = content[ctx->header.out_e];
  content[ctx->header.out_e] = '\0';
#endif
  ctx->out_index.keyword = (const char *)
      &content[hctx->out_e - hctx->out_matched_index->length];
  ctx->out_index.extra = extra;

  return true;
}

bool dist_next_on_index(dist_context_t ctx) {
  // alias
  unsigned char *content = ctx->header.content;
  context_t hist = ctx->_history_context;
  context_t hctx = ctx->_head_context;
  context_t tctx = ctx->_tail_context;
  context_t dctx = ctx->_digit_context;

#ifdef REPLACE_BY_ZERO
  content[ctx->header.out_e] = ctx->_c;  // recover content
#endif

  switch (ctx->_state) {
    case dist_match_state_new_round: break;
    case dist_match_state_check_history: goto check_history;
    case dist_match_state_check_tail: goto check_tail;
    case dist_match_state_check_prefix: goto check_prefix;
  }

  while (dat_ac_next_on_index((dat_context_t) hctx)) {
    // check number
    size_t dist = (size_t) hctx->out_matched_index->extra;
    if (dist & DIGITAL_MASK) {
      ctx->_state = dist_match_state_check_prefix;
      dist &= ~DIGITAL_MASK;
      // skip number
      size_t tail_so = hctx->out_e;
      while (dist--) {
        tail_so++;
        if (!number_bitmap[content[tail_so]])
          break;
      }
      // check tail
      matcher_reset_context(dctx, &content[tail_so], ctx->header.len - tail_so);
check_prefix:
      while (dat_prefix_next_on_index((dat_context_t) dctx)) {
        if (dctx->out_matched_index->_tag == hctx->out_matched_index->_tag)
          return dist_construct_out(ctx,
                                    dctx->out_matched_index->extra,
                                    tail_so + dctx->out_e);
      }
      continue;
    }

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
      if (distance > MAX_WORD_DISTANCE) {  // max distance is 15
        if (hist[ctx->_i].out_e - hctx->out_e
            > MAX_CHAR_DISTANCE + ctx->_matcher->_dict->max_extra_length)
          goto next_round;
        continue;
      }

      match_dict_index_t matched_index = hist[ctx->_i].out_matched_index;
      for (; matched_index != NULL; matched_index = matched_index->_next) {
        if (matched_index->_tag <= hctx->out_matched_index->_tag) break;
      }
      if (matched_index != NULL
          && matched_index->_tag == hctx->out_matched_index->_tag) {
        if (distance <= (size_t) hctx->out_matched_index->extra)
          return dist_construct_out(ctx,
                                    matched_index->extra,
                                    hist[ctx->_i].out_e);
        break;
      }
    }

    ctx->_state = dist_match_state_check_tail;
check_tail:
    // one node will match the tag only once.
    while (dat_ac_next_on_node((dat_context_t) tctx)) {
      long diff_pos =
          utf8_word_distance(ctx->_utf8_pos, hctx->out_e, tctx->out_e);
      long distance = (long) (diff_pos - tctx->out_matched_index->wlen);
      if (distance < 0) continue;

      // record history
      hist[ctx->_htidx] = *tctx;
      ctx->_htidx = (ctx->_htidx + 1) % HISTORY_SIZE;
      ctx->_hcnt++;

      if (distance > MAX_WORD_DISTANCE) {  // max distance is 15
        if (tctx->out_e - hctx->out_e >
            MAX_CHAR_DISTANCE + ctx->_matcher->_dict->max_extra_length)
          goto next_round;
        continue;
      }

      match_dict_index_t matched_index = tctx->out_matched_index;
      for (; matched_index != NULL; matched_index = matched_index->_next) {
        // link table's tag is descending order
        if (matched_index->_tag <= hctx->out_matched_index->_tag) break;
      }
      if (matched_index != NULL
          && matched_index->_tag == hctx->out_matched_index->_tag) {
        if (distance <= (size_t) hctx->out_matched_index->extra)
          return dist_construct_out(ctx, matched_index->extra, tctx->out_e);
        break;
      }
    }
next_round:
    ctx->_state = dist_match_state_new_round;
  }

  return false;
}
