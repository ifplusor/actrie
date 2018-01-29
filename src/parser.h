/**
 * parser.h - syntactic analysis for matcher vocabulary pattern
 */

#ifndef _ACTRIE_PARSER_H_
#define _ACTRIE_PARSER_H_

#include "vocab.h"
#include "pattern.h"
#include "tokenizer.h"

ptrn_t parse_pattern(strlen_t pattern);
void *parse_vocab(vocab_t vocab);

#endif //_ACTRIE_PARSER_H_
