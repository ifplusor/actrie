//
// Created by james on 1/22/18.
//

#include "pattern.h"

void ptrn_clean(aobj id);

ameta(ptrn,
  FOUR_CHARS_TO_INT('P', 'T', 'R', 'N'),
  ptrn_clean
)

aobj ptrn_init(void *ptr, void *data) {
  ptrn_t id = aobj_init(ptrn, ptr);
  if (id) {
    id->type = ptrn_type_empty;
    id->desc = NULL;
  }
  return id;
}

void ptrn_clean(aobj id) {
  if (TAGGED_AOBJECT(id)) {
    ptrn_t ptrn = id;
    switch (ptrn->type) {
      case ptrn_type_dist:
        afree(ptrn->desc);
        break;
      default: // 默认 ptrn->desc 也是 aobj
        _release(ptrn->desc);
        break;
    }
  }
}


// pure pattern
// ===========================

afunc_defn(ptrn, pure, aobj, dstr_t text) {
  ptrn_t ptrn = aobj_alloc(ptrn_s, ptrn_init);
  if (ptrn != NULL) {
    ptrn->type = ptrn_type_pure;
    _retain(text);
    ptrn->desc = text;
  }
  return ptrn;
}


// alternative pattern
// ===========================

/**
 * ptrn_cat - concatenate two patterns.
 *
 * after call this function, 'before' and 'after' object will be released,
 * a new merge object will be generated.
 */
afunc_defn(ptrn, cat, aobj, ptrn_t before, ptrn_t after) {
  if (before == NULL || after == NULL) return NULL;

  if (before->type == ptrn_type_alter) {
    // reuse before
    list_t con = before->desc;
    while (con->cdr != NULL) con = con->cdr; // walk to tail
    if (after->type == ptrn_type_alter) {
      _retain(after->desc);
      con->cdr = after->desc;
      _release(after);
    } else {
      con->cdr = _alloc(list, cons, after, NULL);
      _release(after);
    }
    return before;
  } else {
    if (after->type == ptrn_type_alter) {
      // reuse after
      ptrn_t ptrn = after->desc;
      after->desc = _alloc(list, cons, before, ptrn);
      _release(ptrn);
      _release(before);
      return after;
    } else {
      // alloc new ptrn
      ptrn_t ptrn = aobj_alloc(ptrn_s, ptrn_init);
      if (ptrn != NULL) {
        ptrn->type = ptrn_type_alter;
        list_t con = _alloc(list, cons, after, NULL);
        ptrn->desc = _alloc(list, cons, before, con);
        _release(con);
        _release(before);
        _release(after);
      }
      return ptrn;
    }
  }
}


// ambiguous pattern
// ===========================

afunc_defn(ptrn, ambi, aobj, ptrn_t origin, ptrn_t ambi) {
  ptrn_t ptrn = aobj_alloc(ptrn_s, ptrn_init);
  if (ptrn != NULL) {
    ptrn->type = ptrn_type_ambi;
    ptrn->desc = _alloc(list, cons, origin, ambi);
    _release(origin);
    _release(ambi);
  }
  return ptrn;
}


// antonym pattern
// ===========================

afunc_defn(ptrn, anto, aobj, ptrn_t origin, ptrn_t anto) {
  ptrn_t ptrn = aobj_alloc(ptrn_s, ptrn_init);
  if (ptrn != NULL) {
    ptrn->type = ptrn_type_anto;
    ptrn->desc = _alloc(list, cons, origin, anto);
    _release(origin);
    _release(anto);
  }
  return ptrn;
}

// distance pattern
// ===========================

afunc_defn(ptrn, dist, aobj, ptrn_t head, ptrn_t tail, int min, int max) {
  ptrn_t ptrn = aobj_alloc(ptrn_s, ptrn_init);
  if (ptrn != NULL) {
    ptrn->type = ptrn_type_dist;
    pdd_t pdd = amalloc(sizeof(pdd_s));
    pdd->head = head;
    pdd->tail = tail;
    pdd->min = min;
    pdd->max = max;
    ptrn->desc = pdd;
  }
  return ptrn;
}

