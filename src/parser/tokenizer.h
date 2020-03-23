/**
 * tokenizer.h - lexical analysis for matcher vocabulary pattern
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#ifndef __ACTRIE_TOKENIZER_H__
#define __ACTRIE_TOKENIZER_H__

#include <alib/io/stream.h>
#include <alib/object/dstr.h>

#define TOKEN_TEXT (0)
#define TOKEN_EOF (-1)
#define TOKEN_ERR (-2)
#define TOKEN_SUBS (-3)
#define TOKEN_SUBE (-4)
#define TOKEN_AMBI (-5)
#define TOKEN_ANTO (-6)
#define TOKEN_ANY (-7)
#define TOKEN_NUM (-8)
#define TOKEN_REPT (-9)
#define TOKEN_ALT (-10)

int token_next(stream_t stream, dstr_t* token);
int token_rept_min();
int token_rept_max();

#endif  // __ACTRIE_TOKENIZER_H__
