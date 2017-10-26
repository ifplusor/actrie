//
// Created by james on 10/23/17.
//

#include "configure.h"
#include "dict0.h"

void mcnf_clean(aobj id);

static aobj_meta_s mcnf_meta = {
    .isa = FOUR_CHARS_TO_INT('M', 'C', 'N', 'F'),
    .refcnt = 0,
    .clean = mcnf_clean,
};

aobj mcnf_init(void *ptr, void *data) {
  aobj id = aobj_init(ptr, &mcnf_meta);
  if (id) {
    matcher_config_t mcnf = GET_AOBJECT(id);
    mcnf->clean = NULL;
  }
  return id;
}

void mcnf_clean(aobj id) {
  if (TAGGED_AOBJECT(id)) {
    matcher_config_t mcnf = GET_AOBJECT(id);
    if (mcnf->clean != NULL)
      mcnf->clean(mcnf);
  }
}

aobj matcher_conf(uint8_t id, matcher_type_e type,
                  dict_add_index_func add_index, size_t extra) {
  aobj conf = aobj_alloc_with_ex(matcher_config_s, mcnf_init, NULL, extra);
  if (conf != NULL) {
    matcher_config_t config = GET_AOBJECT(conf);
    config->id = id;
    config->type = type;
    config->add_index = add_index;
  }
  return conf;
}

aobj matcher_root_conf(uint8_t id) {
  return matcher_conf(id, matcher_type_stub, dict_add_index, 0);
}

void stub_config_clean(matcher_config_t config) {
  if (config != NULL) {
    stub_config_t stub_config = (stub_config_t) config->buf;
    _release(stub_config->stub);
  }
}

aobj matcher_wordattr_conf(uint8_t id, aobj stub) {
  aobj conf = matcher_conf(id, matcher_type_wordattr, dict_add_wordattr_index,
                           sizeof(stub_config_s));
  if (conf) {
    matcher_config_t config = GET_AOBJECT(conf);
    config->clean = stub_config_clean;
    stub_config_t stub_config = (stub_config_t) config->buf;
    stub_config->stub = stub;
  }
  return conf;
}

aobj matcher_alternation_conf(uint8_t id, aobj stub) {
  aobj conf = matcher_conf(id, matcher_type_alteration,
                           dict_add_alternation_index, sizeof(stub_config_s));
  if (conf) {
    matcher_config_t config = GET_AOBJECT(conf);
    config->clean = stub_config_clean;
    stub_config_t stub_config = (stub_config_t) config->buf;
    stub_config->stub = stub;
  }
  return conf;
}
