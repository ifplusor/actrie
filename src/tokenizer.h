/**
 * tokenizer.h - lexical analysis for matcher vocabulary pattern
 */

#ifndef _ACTRIE_TOKENIZER_H_
#define _ACTRIE_TOKENIZER_H_

#include <stream.h>
#include <obj/dstr.h>

#define TOKEN_TXT  (0)
#define TOKEN_EOF  (-1)
#define TOKEN_ERR  (-2)
#define TOKEN_ALT  (-3)
#define TOKEN_SUBS (-4)
#define TOKEN_SUBE (-5)
#define TOKEN_AMBI (-6)
#define TOKEN_ANTO (-7)
#define TOKEN_DIST (-8)

int token_next(stream_t stream, dstr_t *token);
int token_min_dist();
int token_max_dist();

#endif //_ACTRIE_TOKENIZER_H_
