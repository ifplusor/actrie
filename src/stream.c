//
// Created by James on 2017/9/30.
//

#include "stream.h"


bool stream_destruct(stream_t self) {
  if (self == NULL) return false;
  return self->_func->sf_destruct(self);
}

int stream_getc(stream_t self) {
  return self->_func->sf_getc(self);
}

int stream_ungetc(stream_t self, int c) {
  return self->_func->sf_ungetc(self, c);
}

void stream_skip_bom(stream_t self) {
  // filter bom
  int ch;
  if ((ch = stream_getc(self)) != 0xef) {
    if (ch != EOF) stream_ungetc(self, ch);
  } else if ((ch = stream_getc(self)) != 0xbb) {
    if (ch != EOF) stream_ungetc(self, ch);
    stream_ungetc(self, 0xef);
  } else if ((ch = stream_getc(self)) != 0xbf) {
    if (ch != EOF) stream_ungetc(self, ch);
    stream_ungetc(self, 0xbb);
    stream_ungetc(self, 0xef);
  }
}

void stream_rewind(stream_t self) {
  self->_func->sf_rewind(self);
  stream_skip_bom(self);
}

bool file_stream_destruct(stream_t self) {
  if (self == NULL) return false;
  file_stream_t fs = (file_stream_t) self;
  fclose(fs->fp);
  free(fs);
  return true;
}

int file_stream_getc(stream_t self) {
  file_stream_t fs = (file_stream_t) self;
  return getc(fs->fp);
}

int file_stream_ungetc(stream_t self, int c) {
  file_stream_t fs = (file_stream_t) self;
  return ungetc(c, fs->fp);
}

void file_stream_rewind(stream_t self) {
  file_stream_t fs = (file_stream_t) self;
  rewind(fs->fp);
}

const stream_func_l file_stream_func = {
    .sf_destruct = file_stream_destruct,
    .sf_getc = file_stream_getc,
    .sf_ungetc = file_stream_ungetc,
    .sf_rewind = file_stream_rewind,
};

stream_t file_stream_construct(const char *path) {
  FILE *fp = NULL;
  do {
    fp = fopen(path, "rb");
    if (fp == NULL) break;

    file_stream_t stream = malloc(sizeof(file_stream_s));
    if (stream == NULL) break;

    stream->header._func = &file_stream_func;
    stream->fp = fp;

    return (stream_t) stream;
  } while(0);
  if (fp != NULL) fclose(fp);
  return NULL;
}

bool string_stream_destruct(stream_t self) {
  if (self == NULL) return false;
  string_stream_t ss = (string_stream_t) self;
  free(ss->str);
  free(ss);
  return false;
}

int string_stream_getc(stream_t self) {
  string_stream_t ss = (string_stream_t) self;
  return striter_next(&ss->iter);
}

int string_stream_ungetc(stream_t self, int c) {
  string_stream_t ss = (string_stream_t) self;
  if (ss->iter.cur <= 0) return EOF;
  return ss->iter.ptr[--ss->iter.cur] = (unsigned char) c;
}

void string_stream_rewind(stream_t self) {
  string_stream_t ss = (string_stream_t) self;
  striter_reset(&ss->iter);
}

const stream_func_l string_stream_func = {
    .sf_destruct = string_stream_destruct,
    .sf_getc = string_stream_getc,
    .sf_ungetc = string_stream_ungetc,
    .sf_rewind = string_stream_rewind,
};

stream_t string_stream_construct(const char *str) {
  char *dup = NULL;
  do {
    dup = strdup(str);
    if (dup == NULL) break;

    string_stream_t stream = malloc(sizeof(string_stream_s));
    if (stream == NULL) break;

    stream->header._func = &string_stream_func;
    stream->str = dup;
    striter_init(&stream->iter, dup, strlen(dup));

    return (stream_t) stream;
  } while(0);
  if (dup != NULL) free(dup);
  return NULL;
}


stream_t stream_construct(stream_type_e type, const char *src) {
  stream_t stream = NULL;
  if (type == stream_type_file) {
    stream = file_stream_construct(src);
  } else if (type == stream_type_string) {
    stream = string_stream_construct(src);
  }
  if (stream != NULL) stream_skip_bom(stream);
  return stream;
}
