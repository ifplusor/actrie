/**
 * pattern.h
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#ifndef __ACTRIE_PATTERN_H__
#define __ACTRIE_PATTERN_H__

#include <alib/object/aobj.h>
#include <alib/object/dstr.h>
#include <alib/object/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum _ptrn_type_ {
  ptrn_type_empty = 0,
  ptrn_type_pure,
  ptrn_type_anti_ambi,
  ptrn_type_anti_anto,
  ptrn_type_dist,
  ptrn_type_alter
} ptrn_type_e;

typedef enum _ptrn_dist_type_ { ptrn_dist_type_any = 0, ptrn_dist_type_num } ptrn_dist_type_e;

// clang-format off
aclass(ptrn,
  ptrn_type_e type;
  void* desc;
);
// clang-format on

typedef struct _ptrn_dist_desc_ {
  ptrn_t head;
  ptrn_t tail;
  ptrn_dist_type_e type;
  int min, max;
} pdd_s, *pdd_t;

afunc_delc(ptrn, pure, aobj, dstr_t text);
afunc_delc(ptrn, cat, aobj, ptrn_t before, ptrn_t after);
afunc_delc(ptrn, ambi, aobj, ptrn_t origin, ptrn_t ambi);
afunc_delc(ptrn, anto, aobj, ptrn_t origin, ptrn_t anto);
afunc_delc(ptrn, dist, aobj, ptrn_t head, ptrn_t tail, ptrn_dist_type_e type, int min, int max);

#endif  // __ACTRIE_PATTERN_H__
