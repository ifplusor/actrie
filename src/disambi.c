//
// Created by james on 9/27/17.
//

#include <regex.h>
#include "disambi.h"

const matcher_func ambi_matcher_func = {
    .destruct = (matcher_destruct_func) ambi_destruct,
    .alloc_context = (matcher_alloc_context_func) ambi_alloc_context
};

const context_func dist_context_func = {
    .free_context = (matcher_free_context_func) ambi_free_context,
    .reset_context = (matcher_reset_context_func) ambi_reset_context,
    .next = (matcher_next_func) ambi_next_on_index
};

/*
 * pattern will match:
 * - A(?&!D1|D2)
 */
static const char
    *pattern = "([^()]*?)(\\(\\?&!(.*?)\\))?";
static regex_t reg;
static bool pattern_compiled = false;
static const char *tokens_delimiter = "|";

bool dict_disambi_add_keyword_and_extra(match_dict_t dict,
                                        char keyword[],
                                        char extra[]) {
  size_t key_cur, head_cur, tail_cur;
  size_t tag, distance;
  regmatch_t pmatch[4];
  char *split;
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
    err = regexec(&reg, keyword, 4, pmatch, 0);
    if (err == REG_NOMATCH) {
      return true;
    } else if (err != REG_NOERROR) {
      return false;
    }

    pmatch[1]; // keyword
    pmatch[3]; // ambi

  }

  return true;
}

ambi_matcher_t dist_construct(match_dict_t dict, bool enable_automation) {
  ambi_matcher_t matcher;

  trie_t trie;


  head_trie = dist_trie_filter_construct(dict, match_dict_keyword_type_head);
  if (head_trie == NULL) return NULL;

  tail_trie = dist_trie_filter_construct(dict, match_dict_keyword_type_tail);
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

ambi_matcher_t ambi_construct_by_file(const char *path, bool enable_automation) {
  return NULL;
}

ambi_matcher_t ambi_construct_by_string(const char *string,
                                        bool enable_automation) {
  return NULL;
}

bool ambi_destruct(ambi_matcher_t self) {
  return false;
}

ambi_context_t ambi_alloc_context(ambi_matcher_t matcher) {
  return NULL;
}

bool ambi_free_context(ambi_context_t context) {
  return false;
}

bool ambi_reset_context(ambi_context_t context,
                        unsigned char content[],
                        size_t len) {
  return false;
}

bool ambi_next_on_index(ambi_context_t ctx) {
  return false;
}

//int main() {
//  size_t key_cur, head_cur, tail_cur;
//  size_t tag, distance;
//  regmatch_t pmatch[12];
//  char dist[3], *split;
//  int err;
//
//  if (!pattern_compiled) {
//    // compile pattern
//    err = regcomp(&reg, pattern, REG_EXTENDED | REG_NEWLINE);
//    if (err) {
//      fprintf(stderr, "error: compile regex failed!\n");
//      return false;
//    } else {
//      pattern_compiled = true;
//    }
//  }
//
//  err = regexec(&reg, "1234(?&!hhh|aaa)", 12, pmatch, 0);
//  if (err == REG_NOMATCH) {
//    return true;
//  } else if (err != REG_NOERROR) {
//    return false;
//  }
//
//  return 0;
//}
