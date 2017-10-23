//
// Created by james on 10/23/17.
//

#include "configure.h"
#include "dict0.h"

matcher_config_t matcher_stub_config(uint8_t id, matcher_config_t stub) {
  matcher_config_t config = amalloc(sizeof(matcher_config_s));
  if (config != NULL) {
    config->id = id;
    config->type = matcher_type_stub;
    config->add_index = dict_add_index;
    config->config = NULL;
  }
  return config;
}

matcher_config_t matcher_wordattr_config(uint8_t id, matcher_config_t stub) {
  matcher_config_t config = amalloc(sizeof(matcher_config_s));
  if (config != NULL) {
    config->id = id;
    config->type = matcher_type_wordattr;
    config->add_index = dict_add_wordattr_index;
    config->config = stub;
  }
  return config;
}

matcher_config_t matcher_alternation_config(uint8_t id, matcher_config_t stub) {
  matcher_config_t config = amalloc(sizeof(matcher_config_s));
  if (config != NULL) {
    config->id = id;
    config->type = matcher_type_alteration;
    config->add_index = dict_add_alternation_index;
    config->config = stub;
  }
  return config;
}

