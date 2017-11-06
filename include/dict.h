#ifndef _ACTRIE_DICT_H_
#define _ACTRIE_DICT_H_

#include <acom.h>
#include <obj/dynastr.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define mdi_prop_empty = (0x00)
#define mdi_prop_reserved (0x01)

// store attr
#define mdi_prop_bufkey (0x01 << 1)
#define mdi_prop_bufextra (0x01 << 2)

// word attr
#define mdi_prop_word (0x01 << 3)
#define mdi_prop_alnum (0x01 << 4)

// ambi attr
#define mdi_prop_clearly (0x01 << 5)
#define mdi_prop_normal (0x01 << 6)
#define mdi_prop_ambi (0x01 << 7)

// dist attr
#define mdi_prop_single (0x01 << 8)
#define mdi_prop_head (0x01 << 9)
#define mdi_prop_tail (0x01 << 10)
#define mdi_prop_dist_digit (0x01 << 11)

// post process attr
#define mdi_prop_tag_id (0x01 << 12)

#define mdi_prop_matcher_mask 0xFF
#define mdi_prop_get_matcher(prop) (((prop) >> 16) & mdi_prop_matcher_mask)
#define mdi_prop_set_matcher(prop, matcher) ((prop) | (((matcher) & mdi_prop_matcher_mask) << 16))

typedef uint32_t mdi_prop_f;

/*
 * optimize for cache:
 *      _tag   _keyword  _extra   length    wlen     prop
 *   +--------+--------+--------+--------+--------+--------+
 *   | 8 byte | 8 byte | 8 byte | 2 byte | 2 byte | 4 byte |
 *   +--------+--------+--------+--------+--------+--------+
 */
typedef struct match_dict_index {
  // TODO: set magic?
  void *_tag; // place in first for compare
  ds keyword;                   /* 匹配的正文文本 */
  ds extra;
  uint16_t length;              /* 文本的字符长度 */
  uint16_t wlen;                /* 文本的字节长度 */
  mdi_prop_f prop;
} mdi_s, *mdi_t;

#define PCRE_VEC_SO(vector, index) ((vector)[(index)*2])
#define PCRE_VEC_EO(vector, index) ((vector)[(index)*2+1])

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ACTRIE_DICT_H_ */
