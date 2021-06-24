/**
 * pattern.c
 *
 * @author James Yin <ywhjames@hotmail.com>
 */
#include "pattern.h"

void ptrn_clean(aobj id);

// clang-format off
ameta(ptrn,
  FOUR_CHARS_TO_INT('P', 'T', 'R', 'N'),
  ptrn_clean
);
// clang-format on

aobj ptrn_init(void* ptr, void* data) {
  ptrn_t id = aobj_init(ptrn, ptr);
  if (id) {
    id->type = ptrn_type_empty;
    id->desc = NULL;
    id->notation = NULL;
  }
  return id;
}

void ptrn_clean(aobj id) {
  if (TAGGED_AOBJECT(id)) {
    ptrn_t ptrn = id;
    _release(ptrn->notation);
    switch (ptrn->type) {
      case ptrn_type_dist: {
        pdd_t pdd = ptrn->desc;
        _release(pdd->head);
        _release(pdd->tail);
        afree(pdd);
        break;
      }
      default: {
        // 默认 ptrn->desc 也是 aobj
        _release(ptrn->desc);
        break;
      }
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
    _retain(text);
    ptrn->notation = text;
  }
  return ptrn;
}

// alternative pattern
// ===========================

/**
 * ptrn_cat - concatenate two patterns.
 *
 * after call this function, 'before' and 'after' object will be released,
 * a new ptrn_cat object will be generated.
 */
afunc_defn(ptrn, cat, aobj, ptrn_t before, ptrn_t after) {
  if (before == NULL || after == NULL) {
    return NULL;
  }

  // TODO: sort, and unique

  ptrn_t ptrn = NULL;
  if (before->type == ptrn_type_alter) {
    // reuse before
    list_t con = before->desc;
    while (con->cdr != NULL) {
      con = con->cdr;  // walk to tail
    }
    if (after->type == ptrn_type_alter) {
      // concat
      _retain(after->desc);
      con->cdr = after->desc;
      _release(after);
    } else {
      // append
      con->cdr = _alloc(list, cons, after, NULL);
      _release(after);
    }
    ptrn = before;
  } else if (after->type == ptrn_type_alter) {
    // reuse after
    list_t con = after->desc;
    // prepend
    after->desc = _alloc(list, cons, before, con);
    _release(con);
    _release(before);
    ptrn = after;
  } else {
    // alloc new ptrn
    ptrn = aobj_alloc(ptrn_s, ptrn_init);
    if (ptrn != NULL) {
      ptrn->type = ptrn_type_alter;
      list_t con = _alloc(list, cons, after, NULL);
      ptrn->desc = _alloc(list, cons, before, con);
      _release(con);
      _release(before);
      _release(after);
    }
  }
  // TODO: set notation
  return ptrn;
}

// ambiguity pattern
// ===========================

/**
 * ptrn_ambi -
 *
 * after call this function, 'center' and 'ambi' object will be released,
 * a new ptrn_ambi object will be generated.
 */
afunc_defn(ptrn, ambi, aobj, ptrn_t center, ptrn_t ambi) {
  ptrn_t ptrn = aobj_alloc(ptrn_s, ptrn_init);
  if (ptrn != NULL) {
    ptrn->type = ptrn_type_anti_ambi;
    ptrn->desc = _alloc(list, cons, center, ambi);
    _release(center);
    _release(ambi);
    // TODO: set notation
  }
  return ptrn;
}

// antonym pattern
// ===========================

/**
 * ptrn_anto -
 *
 * after call this function, 'center' and 'anto' object will be released,
 * a new ptrn_anto object will be generated.
 */
afunc_defn(ptrn, anto, aobj, ptrn_t center, ptrn_t anto) {
  ptrn_t ptrn = aobj_alloc(ptrn_s, ptrn_init);
  if (ptrn != NULL) {
    ptrn->type = ptrn_type_anti_anto;
    ptrn->desc = _alloc(list, cons, center, anto);
    _release(center);
    _release(anto);
    // TODO: set notation
  }
  return ptrn;
}

// distance pattern
// ===========================

/**
 * ptrn_dist -
 *
 * after call this function, 'head' and 'tail' object will be released,
 * a new ptrn_dist object will be generated.
 */
afunc_defn(ptrn, dist, aobj, ptrn_t head, ptrn_t tail, ptrn_dist_type_e type, int min, int max) {
  ptrn_t ptrn = aobj_alloc(ptrn_s, ptrn_init);
  if (ptrn != NULL) {
    ptrn->type = ptrn_type_dist;
    pdd_t pdd = amalloc(sizeof(pdd_s));
    pdd->head = head;
    pdd->tail = tail;
    pdd->type = type;
    pdd->min = min;
    pdd->max = max;
    ptrn->desc = pdd;
    // TODO: set notation
  }
  return ptrn;
}
