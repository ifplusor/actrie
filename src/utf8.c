//
// Created by james on 4/27/17.
//

#include "utf8.h"

unsigned char utf8_size_table[256] =
    {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 1, 1
    };
#define UTF8_WORD_SIZE(s) (utf8_size_table[(unsigned char)*(s)])

size_t utf8_word_length(const char *s, size_t slen) {
  size_t wlen = 0;
  size_t i = 0;
  while (i < slen) {
    i += UTF8_WORD_SIZE(s + i);
    wlen++;
  }

  return wlen;
}

/**
 * utf8_word_position - 取得utf8编码字符串的字位置
 * @param s 	string buffer
 * @param wp 	the size of buffer wp must bigger than slen
 * @param slen 	length of string in buffer s
 * @return
 */
int utf8_word_position(const char *in_s, size_t in_slen, size_t *out_wp) {
  size_t wlen = 0;
  size_t i = 0;
  while (i < in_slen) {
    switch (UTF8_WORD_SIZE(in_s + i)) {
      case 6: if (i >= in_slen) break;
        out_wp[i++] = wlen;
      case 5: if (i >= in_slen) break;
        out_wp[i++] = wlen;
      case 4: if (i >= in_slen) break;
        out_wp[i++] = wlen;
      case 3: if (i >= in_slen) break;
        out_wp[i++] = wlen;
      case 2: if (i >= in_slen) break;
        out_wp[i++] = wlen;
      case 1: if (i >= in_slen) break;
        out_wp[i++] = wlen;
      default:break;
    }
    wlen++;
  }
  out_wp[i] = wlen; // upper bound
  return 0;
}

