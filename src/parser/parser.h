/**
 * parser.h - syntactic analysis for matcher vocabulary pattern
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#ifndef __ACTRIE_PARSER_H__
#define __ACTRIE_PARSER_H__

#include "../pattern.h"
#include "../vocab.h"

ptrn_t parse_pattern(strlen_t pattern);

typedef void (*have_pattern_f)(ptrn_t pattern, strlen_t extra, void* arg);

bool parse_vocab(vocab_t vocab, have_pattern_f have_pattern, void* arg, bool ignore_bad_pattern, bool bad_as_plain);

#endif  // __ACTRIE_PARSER_H__
