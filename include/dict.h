#ifndef _ACTRIE_DICT_H_
#define _ACTRIE_DICT_H_

#include "acom.h"
#include "dynastr.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum match_dict_index_prop {
  mdi_prop_empty = 0x00,
  mdi_prop_reserved = 0x01,

  // store attr
  mdi_prop_bufkey = 0x01 << 1,
  mdi_prop_bufextra = 0x01 << 2,

  // word attr
  mdi_prop_word = 0x01 << 3,
  mdi_prop_alnum = 0x01 << 4,

  // ambi attr
  mdi_prop_normal = 0x01 << 5,
  mdi_prop_ambi = 0x01 << 6,

  // dist attr
  mdi_prop_single = 0x01 << 7,
  mdi_prop_head = 0x01 << 8,
  mdi_prop_tail = 0x01 << 9,
  mdi_prop_dist_digit = 0x01 << 10,

  // post process attr
  mdi_prop_tag_id = 0x01 << 11,

} mdi_prop_f;

typedef struct mdi_prop_bitmap {
  int reserved : 1;

  int bufkey : 1;
  int bufextra : 1;

  int word : 1;
  int alnum : 1;

  int normal : 1;
  int ambi : 1;

  int single : 1;
  int head : 1;
  int tail : 1;
  int dist_digit : 1;

  int tag_id : 1;

  int placeholder : 24;
} mdi_prop_bp;

/*
 * optimize for cache:
 *      _tag   _keyword  _extra   length    wlen     prop
 *   +--------+--------+--------+--------+--------+--------+
 *   | 8 byte | 8 byte | 8 byte | 2 byte | 2 byte | 4 byte |
 *   +--------+--------+--------+--------+--------+--------+
 */
typedef struct match_dict_index {
  void *_tag; // place in first for compare
  ds _keyword;            /* 匹配的正文文本 */
#define mdi_keyword _keyword
  ds _extra;
#define mdi_extra _extra
  uint16_t length;              /* 文本的字符长度 */
  uint16_t wlen;                /* 文本的字节长度 */
  mdi_prop_f prop;
} mdi_s, *mdi_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ACTRIE_DICT_H_ */
