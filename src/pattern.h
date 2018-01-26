//
// Created by james on 1/22/18.
//

#ifndef _ACTRIE_PATTERN_H_
#define _ACTRIE_PATTERN_H_

#include <obj/aobj.h>
#include <obj/dstr.h>
#include <obj/list.h>


typedef enum ptrn_type {
  ptrn_type_empty = 0,
  ptrn_type_pure,
  ptrn_type_ambi,
  ptrn_type_anto,
  ptrn_type_dist,
  ptrn_type_alter
} ptrn_type_e;

aclass(ptrn,
  ptrn_type_e type;
  void *desc;
)

typedef struct _ptrn_dist_desc {
  ptrn_t head;
  ptrn_t tail;
  int min, max;
} pdd_s, *pdd_t;

afunc_delc(ptrn, pure, aobj, dstr_t text);
afunc_delc(ptrn, cat, aobj, ptrn_t before, ptrn_t after);
afunc_delc(ptrn, ambi, aobj, ptrn_t origin, ptrn_t ambi);
afunc_delc(ptrn, anto, aobj, ptrn_t origin, ptrn_t anto);
afunc_delc(ptrn, dist, aobj, ptrn_t head, ptrn_t tail, int min, int max);

#endif //_ACTRIE_PATTERN_H_
