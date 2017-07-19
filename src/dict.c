#include "dict.h"
#include "utf8.h"

const bool alpha_number_bitmap[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const bool alpha_bitmap[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const bool number_bitmap[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

bool dict_default_add_keyword_and_extra(match_dict_ptr dict,
                                        char keyword[],
                                        char extra[]);

match_dict_ptr dict_alloc()
{
  match_dict_ptr p = (match_dict_ptr) malloc(sizeof(match_dict));
  if (p == NULL) return NULL;

  p->index = NULL;
  p->buffer = NULL;
  p->_ref_count = 1;

  p->add_keyword_and_extra = dict_default_add_keyword_and_extra;
  p->before_reset = NULL;

  return p;
}

match_dict_ptr dict_assign(match_dict_ptr dict)
{
  if (dict != NULL) {
    dict->_ref_count++;
  }

  return dict;
}

void dict_release(match_dict_ptr dict)
{
  if (dict != NULL) {
    dict->_ref_count--;
    if (dict->_ref_count == 0) {
      if (dict->index != NULL)
        free(dict->index);
      if (dict->buffer != NULL)
        free(dict->buffer);
      free(dict);
    }
  }
}

bool dict_reset(match_dict_ptr dict, size_t index_count, size_t buffer_size)
{
  if (dict->before_reset != NULL)
    dict->before_reset(dict, &index_count, &buffer_size);

  // free memory
  if (dict->index != NULL) free(dict->index);
  if (dict->buffer != NULL) free(dict->buffer);

  dict->idx_count = 0;
  dict->idx_size = index_count + 1;
  dict->index =
      (match_dict_index_ptr) malloc(sizeof(match_dict_index) * dict->idx_size);
  if (dict->index == NULL)
    return false;

  dict->buf_size = buffer_size + 3; /* 增加缓存大小, 防止溢出 */
  dict->buffer = (char *) malloc(sizeof(char) * dict->buf_size);
  if (dict->buffer == NULL) {
    free(dict->index);
    return false;
  }

  memset(dict->index, 0, sizeof(match_dict_index) * dict->idx_size);
  memset(dict->buffer, 0, sizeof(char) * dict->buf_size);
  dict->_cursor = 1; /* 逻辑上 dict->buff[0] 是 "" */
  dict->max_key_length = 0;
  dict->max_extra_length = 0;

  return true;
}

void dict_add_index(match_dict_ptr dict,
                    size_t length,
                    char *keyword,
                    char *extra,
                    char *tag,
                    match_dict_keyword_type type)
{
  if (dict->idx_count == dict->idx_size) {
    dict->idx_size += 100;
    dict->index = realloc(dict->index, sizeof(match_dict) * dict->idx_size);
    memset(dict->index + dict->idx_count, 0, sizeof(match_dict) * 100);
  }

  dict->index[dict->idx_count].next = NULL;
  dict->index[dict->idx_count].length = length;
  dict->index[dict->idx_count].wlen = utf8_word_length(keyword, length);
  dict->index[dict->idx_count].keyword = keyword;
  dict->index[dict->idx_count].extra = extra;
  dict->index[dict->idx_count].tag = tag;
  dict->index[dict->idx_count].type = type;
  dict->idx_count++;
}

bool dict_default_add_keyword_and_extra(match_dict_ptr dict,
                                        char keyword[],
                                        char extra[])
{
  match_dict_keyword_type type;
  size_t key_cur, extra_cur, tag_cur;
  size_t i, key_length;

  if (keyword != NULL && keyword[0] != '\0') {
    type = match_dict_keyword_type_alpha_number;

    key_length = strlen(keyword);
    for (i = 0; i < key_length; i++) {
      if (!alpha_number_bitmap[(unsigned char) keyword[i]] &&
          keyword[i] != ' ') {
        type = match_dict_keyword_type_normal;
        break;
      }
    }

    strcpy(dict->buffer + dict->_cursor, keyword);
    key_cur = dict->_cursor;
    dict->_cursor += key_length + 1;

    if (key_length > dict->max_key_length)
      dict->max_key_length = key_length;

    if (extra == NULL || extra[0] == '\0') {
      extra_cur = 0;
      tag_cur = 0;
    } else {
      char *t;
      size_t extra_length = strlen(extra);
      strcpy(dict->buffer + dict->_cursor, extra);
      extra_cur = dict->_cursor;
      dict->_cursor += extra_length + 1;

      t = strstr(extra, SEPARATOR_ID);
      if (t != NULL) {
        extra_length = t - extra;
        tag_cur = extra_cur + extra_length + sizeof(SEPARATOR_ID) - 1;
        dict->buffer[extra_cur + extra_length] = '\0';
      } else {
        tag_cur = 0;
      }

      if (extra_length > dict->max_extra_length)
        dict->max_extra_length = extra_length;
    }

    dict_add_index(dict,
                   key_length,
                   dict->buffer + key_cur,
                   dict->buffer + extra_cur,
                   dict->buffer + tag_cur,
                   type);
  }

  return true;
}

bool dict_parser_line(match_dict_ptr dict, const char *line_buf)
{
  static char keyword[MAX_LINE_SIZE];
  static char extra[MAX_LINE_SIZE];

  int ret = sscanf(line_buf, "%[^\t\r\n]\t%[^\r\n]", keyword, extra);

  /* 如果sscanf未读取key 则不用处理 直接continue */
  if (ret <= 0) {
    keyword[0] = 0;
    return true;
  }

  /* 回车换行符此处特殊处理 \r \n \r\n */
  if (keyword[0] == '\\') {
    /* \r \r\n */
    if (keyword[1] == 'r') {
      /* \r\n */
      if (keyword[2] == '\\' && keyword[3] == 'n' &&
          keyword[4] == '0') {
        keyword[0] = '\r';
        keyword[1] = '\n';
        keyword[2] = 0;
        /* \r */
      } else if (keyword[2] == 0) {
        keyword[0] = '\r';
        keyword[1] = 0;
      }
      /* \n */
    } else if (keyword[1] == 'n' && keyword[2] == 0) {
      keyword[0] = '\n';
      keyword[1] = 0;
    }
  }

  /* 如果不置0 会有上次scanf残留数据 */
  if (ret <= 1) {
    extra[0] = 0;
  }

  /* 将 keyword 和 extra 加入 match_dict */
  return dict->add_keyword_and_extra(dict, keyword, extra);
}

bool dict_parser_by_file(match_dict_ptr dict, FILE *fp)
{
  /* 静态化以后，不支持多线程 */
  static char line_buf[MAX_LINE_SIZE];
  size_t count = 0;

  if (fp == NULL || dict == NULL) {
    return false;
  }

  /* 计算词典条目数 */
  fseek(fp, 0, SEEK_SET);
  while (fgets(line_buf, MAX_LINE_SIZE, fp) != NULL) {
    count++;
  }

  fseek(fp, 0, SEEK_END);
  if (!dict_reset(dict, count, (size_t) ftell(fp) + 5))
    return false;

  fseek(fp, 0, SEEK_SET);

  /* 处理 BOM */
  line_buf[0] = (char) fgetc(fp);
  line_buf[1] = (char) fgetc(fp);
  line_buf[2] = (char) fgetc(fp);
  if ((unsigned char) line_buf[0] != 0xef ||
      (unsigned char) line_buf[1] != 0xbb ||
      (unsigned char) line_buf[2] != 0xbf) {
    ungetc(line_buf[2], fp);
    ungetc(line_buf[1], fp);
    ungetc(line_buf[0], fp);
  }

  while (fgets(line_buf, MAX_LINE_SIZE, fp) != NULL) {
    if (!dict_parser_line(dict, line_buf)) return false;
  }

  return true;
}

const char *split = "\n";

bool dict_parser_by_s(match_dict_ptr dict, const char *s)
{
  char *line_buf;
  size_t count = 0;
  char *work_s, *t_s;

  if (s == NULL || dict == NULL) {
    return false;
  }

  /* 计算词典条目数 */
  work_s = strdup(s);
  for (line_buf = strtok(work_s, split); line_buf;
       line_buf = strtok(NULL, split)) {
    count++;
  }
  free(work_s);

  if (!dict_reset(dict, count, strlen(s) + 1))
    return false;

  work_s = strdup(s);

  /* 处理 BOM */
  if ((unsigned char) work_s[0] == 0xef &&
      (unsigned char) work_s[1] == 0xbb &&
      (unsigned char) work_s[2] == 0xbf) {
    t_s = work_s + 3;
  } else {
    t_s = work_s;
  }

  dict->idx_count = 0;
  for (line_buf = strtok(t_s, split); line_buf;
       line_buf = strtok(NULL, split)) {
    dict_parser_line(dict, line_buf);
  }
  free(work_s);

  return true;
}
