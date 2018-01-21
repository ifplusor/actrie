//
// Created by james on 1/16/18.
//

#include "tokenizer.h"
#include "parser.h"


void *parse_pattern0(stream_t stream) {
  int ch;
  dstr_t token;
  while ((ch = token_next(stream, &token)) != TOKEN_EOF) {
    if (ch == TOKEN_TXT) {

      _release(token);
    } else if (ch == TOKEN_DIST) {
      // dist-pattern
      int dmin = token_min_dist();
      int dmax = token_max_dist();


    } else if (ch == TOKEN_AMBI) {
      // ambi-pattern

    } else if (ch == TOKEN_ANTO) {
      // anto-pattern

    } else if (ch == TOKEN_SUBS) {
      // sub-pattern

    } else if (ch == TOKEN_ALT) {
      // alter-pattern

    } else if (ch == TOKEN_SUBE) {
      //

    } else {
      // error
    }
  }

  return NULL;
}

void *parse_pattern(strlen_t pattern) {
  stream_t stream = stream_construct(stream_type_string, pattern);

  parse_pattern0(stream);

  stream_destruct(stream);

  return NULL;
}

void *parse_vocab(vocab_t vocab) {
  strlen_s keyword, extra;
  vocab_reset(vocab);
  while (vocab_next_word(vocab, &keyword, &extra)) {
    // TODO: parse pattern, output syntax tree
    void *syntax = parse_pattern(&keyword);

  }

  return NULL;
}
