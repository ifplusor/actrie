//
// Created by james on 1/18/18.
//

#include <dynabuf.h>
#include <threadlocal.h>

#include "tokenizer.h"

const bool oct_number_bitmap[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const bool dec_number_bitmap[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const bool hex_number_bitmap[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const int hex_char2num[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,
    0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


tls_key_t dist_key;

bool tokenizer_init() {
  return tls_create_key(&dist_key, free);
}


/**
 * token_oct_num - octal escape sequence
 * @param ch - first char
 */
int token_oct_num(int ch, stream_t stream) {
  int num = ch - '0';
  for (int i = 0; i < 2; i++) {
    ch = stream_getc(stream);
    if (ch == EOF || !oct_number_bitmap[ch])
      return TOKEN_ERR;
    num = num * 8 + (ch - '0');
  }
  return num;
}

/**
 * token_hex_num -  hexadecimal escape sequence
 * @param ch - first char
 */
int token_hex_num(int ch, stream_t stream) {
  int num = ch - '0';
  for (int i = 0; i < 2; i++) {
    ch = stream_getc(stream);
    if (ch == EOF || !hex_number_bitmap[ch])
      return TOKEN_ERR;
    num = num * 16 + hex_char2num[ch];
  }
  return num;
}

/**
 * token_expect - compare next sequence with excepted bytes
 */
bool token_expect(stream_t stream, const uchar *except, size_t len) {
  if (len == 0) return true;
  int ch = stream_getc(stream);
  if (ch != EOF) {
    if (ch == except[0] && token_expect(stream, except + 1, len - 1))
      return true;
    stream_ungetc(stream, ch);
  }
  return false;
}

/**
 * token_expect_char - compare next char with excepted next
 */
bool token_expect_char(stream_t stream, const uchar ch) {
  return token_expect(stream, &ch, 1);
}

/**
 * token_skip_set - skip next sequence when each byte is excepted
 */
bool token_skip_set(stream_t stream, const uchar *except, size_t len) {
  int ch;
  if (except == NULL || len == 0 || except[0] == '\0') {
    // empty set
    return false;
  } else {
    if (len == 1) {
      while ((ch = stream_getc(stream)) != EOF && ch == *except);
    } else {
      uchar table[256];
      uchar *p = memset (table, 0, 64);
      memset (p + 64, 0, 64);
      memset (p + 128, 0, 64);
      memset (p + 192, 0, 64);

      for (int i = 0; i < len; i++) {
        p[except[i]] = 1;
      }

      while ((ch = stream_getc(stream)) != EOF && p[ch]);
    }

    if (ch != EOF)
      stream_ungetc(stream, ch);

    return true;
  }
}

/**
 * token_skip_space - skip next space sequence
 */
bool token_skip_space(stream_t stream) {
  return token_skip_set(stream, (uchar*)" ", 1);
}

bool token_consume_integer(stream_t stream, int *integer) {
  int ch = stream_getc(stream);
  bool is_neg = false;
  if (ch == '-') {
    is_neg = true;
    ch = stream_getc(stream);
  }
  if (ch != EOF && dec_number_bitmap[ch]) {
    int num = 0;
    do {
      num = num * 10 + (ch - '0');
    } while ((ch = stream_getc(stream)) != EOF && dec_number_bitmap[ch]);
    stream_ungetc(stream, ch);

    if (integer != NULL)
      *integer = is_neg ? -num : num;

    return true;
  }
  return false;
}

int token_escape(int ch, stream_t stream) {
  switch (ch) {
    case '\\': return '\\';
    case 't':  return '\t';
    case 'r':  return '\r';
    case 'n':  return '\n';
    case ' ': case '(': case ')': case '.': case '|':
      return ch;
    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7':
      return token_oct_num(ch, stream);
    case 'x':
      return token_hex_num('0', stream);
    default:   return TOKEN_ERR;
  }
}

typedef struct _token_dist_s {
  int min, max;
} dist_s, *dist_t;

void token_set_dist(int min, int max) {
  dist_t dist = tls_get_value(dist_key);
  if (dist == NULL) {
    dist = malloc(sizeof(dist_s));
    tls_set_value(dist_key, dist);
  }
  dist->min = min;
  dist->max = max;
}

int token_min_dist() {
  dist_t dist = tls_get_value(dist_key);
  if (dist == NULL) {
    return -1;
  }
  return dist->min;
}

int token_max_dist() {
  dist_t dist = tls_get_value(dist_key);
  if (dist == NULL) {
    return -1;
  }
  return dist->max;
}

int token_dist(int ch, stream_t stream) {
  // dist-pattern: .{min,max}
  do {
    int min, max;
    if (!token_expect_char(stream, '{')) break;
    token_skip_space(stream);
    if (!token_consume_integer(stream, &min) || min < 0) break;
    token_skip_space(stream);
    if (!token_expect_char(stream, ',')) break;
    token_skip_space(stream);
    if (!token_consume_integer(stream, &max) || max < min) break;
    token_skip_space(stream);
    if (!token_expect_char(stream, '}')) break;

    // set min and max
    token_set_dist(min, max);

    return TOKEN_DIST;
  } while (0);
  return TOKEN_ERR;
}

int token_subs(int ch, stream_t stream) {
  if (token_expect_char(stream, '?')) {
    if (token_expect(stream, (uchar*)"&!", 2)) {
      // ambi-pattern: (?&!pattern)
      return TOKEN_AMBI;
    } else if (token_expect(stream, (uchar*)"<!", 2)) {
      // anto-pattern: (?<!pattern)
      return TOKEN_ANTO;
    }
  }
  return TOKEN_SUBS;
}

int token_meta(int ch, stream_t stream) {
  switch (ch) {
    case '(': // sub-pattern
      return token_subs(ch, stream);
    case ')': return TOKEN_SUBE;
    case '.': // dist-pattern
      return token_dist(ch, stream);
    case '|': return TOKEN_ALT;
    default:  return ch;
  }
}

int token_next(stream_t stream, dstr_t *token) {
  int ch;
  dynabuf_s buffer;

  dynabuf_init(&buffer, 31);

  while ((ch = stream_getc(stream)) != EOF) {
    if (ch == '\\') {
      ch = token_escape(ch, stream);
    } else {
      ch = token_meta(ch, stream);
    }

    if (ch <= TOKEN_ERR)
      break;

    char ch0 = (uchar)ch;
    dynabuf_write(&buffer, &ch0, 1);
  }

  if (ch == EOF) {
    if (dynabuf_empty(&buffer))
      ch = TOKEN_EOF;
    else
      ch = TOKEN_TEXT;
  }

  if (token != NULL) {
    // 出错不返回
    if (dynabuf_empty(&buffer) || ch == TOKEN_ERR) {
      *token = NULL;
    } else {
      strlen_s tok = dynabuf_content(&buffer);
      *token = dstr(&tok);
    }
  }

  dynabuf_clean(&buffer);

  return ch;
}