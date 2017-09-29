//
// Created by james on 9/27/17.
//

#include <regex.h>
#include "disambi.h"

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
