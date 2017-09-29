//
// Created by james on 4/27/17.
//

#ifndef _MATCH_UTF8_H_
#define _MATCH_UTF8_H_

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif


#define utf8_word_distance(wp, s, e) (wp[e] - wp[s])

size_t utf8_word_length(const char *s, size_t len);
int utf8_word_position(const char *in_s, size_t in_slen, size_t *out_wp);


#ifdef __cplusplus
};
#endif

#endif //_MATCH_UTF8_H_
