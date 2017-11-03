//
// Created by james on 10/23/17.
//

#ifndef _ACTRIE_CONFIGURE_H_
#define _ACTRIE_CONFIGURE_H_

#include <obj/aobj.h>
#include <matcher.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct matcher_config;
typedef struct matcher_config *matcher_config_t;

struct match_dict;
typedef struct match_dict *match_dict_t;

typedef bool (*dict_add_index_func)(
    match_dict_t dict, aobj conf, strlen_s keyword, strlen_s extra,
    void *tag, mdi_prop_f prop);

typedef void (*matcher_config_clean)(matcher_config_t config);

typedef struct matcher_config {
  uint8_t id;
  matcher_type_e type;
  dict_add_index_func add_index;
  matcher_config_clean clean;
  char buf[0];
} matcher_config_s;

aobj matcher_conf(uint8_t id, matcher_type_e type,
                  dict_add_index_func add_index, size_t extra);

aobj matcher_root_conf(uint8_t id);

typedef struct stub_config {
  aobj stub;
} stub_config_s, *stub_config_t;

void stub_config_clean(matcher_config_t config);

aobj matcher_wordattr_conf(uint8_t id, aobj stub);
aobj matcher_alternation_conf(uint8_t id, aobj stub);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //_ACTRIE_CONFIGURE_H_
