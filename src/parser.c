//
// Created by james on 1/16/18.
//

#include "parser.h"
#include "vocab.h"


void *parse_pattern(stream_t stream) {


  return NULL;
}

void *parse_vocab(vocab_t vocab) {

  strlen_s keyword, extra;
  vocab_reset(vocab);
  while (vocab_next_word(vocab, &keyword, &extra)) {

    stream_t stream = stream_construct(stream_type_string, &keyword);

    // TODO: parse pattern, output syntax tree
    void *syntax = parse_pattern(stream);

    stream_destruct(stream);

  }

  return NULL;
}
