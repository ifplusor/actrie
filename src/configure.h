//
// Created by james on 10/23/17.
//

#ifndef _ACTRIE_CONFIGURE_H_
#define _ACTRIE_CONFIGURE_H_

#include "dict.h"
#include "matcher.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct matcher_config;
typedef struct matcher_config *matcher_config_t;

struct match_dict;
typedef struct match_dict *match_dict_t;

typedef bool(*dict_add_index_func)
    (match_dict_t dict, matcher_config_t conf, strlen_s keyword,
     strlen_s extra, void *tag, mdi_prop_f prop);

typedef struct matcher_config {
  uint8_t id;
  matcher_type_e type;
  dict_add_index_func add_index;
  void *config;
  char buf[0];
} matcher_config_s;

matcher_config_t matcher_stub_config(uint8_t id, matcher_config_t stub);
matcher_config_t matcher_wordattr_config(uint8_t id, matcher_config_t stub);
matcher_config_t matcher_alternation_config(uint8_t id, matcher_config_t stub);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //_ACTRIE_CONFIGURE_H_
