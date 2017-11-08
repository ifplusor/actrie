//
// Created by james on 10/23/17.
//

#include "configure.h"
#include "dict0.h"

void mcnf_clean(aobj id);

static aobj_meta_s mcnf_meta = {
    .isa = FOUR_CHARS_TO_INT('M', 'C', 'N', 'F'),
    .clean = mcnf_clean,
};

aobj mcnf_init(void *ptr, void *data) {
  matcher_conf_t id = aobj_init(ptr, &mcnf_meta);
  if (id) {
    id->clean = NULL;
  }
  return id;
}

void mcnf_clean(aobj id) {
  if (TAGGED_AOBJECT(id)) {
    matcher_conf_t mcnf = id;
    if (mcnf->clean != NULL)
      mcnf->clean(mcnf);
  }
}

matcher_conf_t matcher_conf(uint8_t id, matcher_type_e type,
                  dict_add_index_func add_index, size_t extra) {
  matcher_conf_t config =
      aobj_alloc_with_ex(matcher_config_s, mcnf_init, NULL, extra);
  if (config != NULL) {
    config->id = id;
    config->type = type;
    config->add_index = add_index;
  }
  return config;
}

matcher_conf_t matcher_root_conf(uint8_t id) {
  return matcher_conf(id, matcher_type_stub, dict_add_index, 0);
}

void stub_config_clean(matcher_conf_t config) {
  if (config != NULL) {
    stub_conf_t stub_config = (stub_conf_t) config->buf;
    _release(stub_config->stub);
  }
}

matcher_conf_t matcher_wordattr_conf(uint8_t id, matcher_conf_t stub) {
  matcher_conf_t conf =
      matcher_conf(id, matcher_type_wordattr, dict_add_wordattr_index,
                   sizeof(stub_conf_s));
  if (conf) {
    conf->clean = stub_config_clean;
    stub_conf_t stub_config = (stub_conf_t) conf->buf;
    stub_config->stub = stub;
  }
  return conf;
}

matcher_conf_t matcher_alternation_conf(uint8_t id, matcher_conf_t stub) {
  matcher_conf_t conf =
      matcher_conf(id, matcher_type_alteration, dict_add_alternation_index,
                   sizeof(stub_conf_s));
  if (conf) {
    conf->clean = stub_config_clean;
    stub_conf_t stub_config = (stub_conf_t) conf->buf;
    stub_config->stub = stub;
  }
  return conf;
}
