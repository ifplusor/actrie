/**
 * wrap.c
 *
 * @author James Yin <ywhjames@hotmail.com>
 *
 * description:
 *   We will parsing arguments and building values in this file for convert
 *   string between python and c.
 *
 *   For parse s,s#:
 *     In python3, Unicode objects are converted to C strings using 'utf-8' encoding.
 *     But in python2, Unicode objects pass back a pointer to the default encoded string
 *       version of the object if such a conversion is possible. All other read-buffer
 *       compatible objects pass back a reference to the raw internal data representation.
 *
 *   For build s#:
 *     In python3, Convert a C string and its length to a Python str object using 'utf-8' encoding.
 *     But in python2, Convert a C string and its length to a Python object.
 */
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "utf8ctx.h"

#if PY_MAJOR_VERSION >= 3
#define IS_PY3K
#endif

PyObject* wrap_construct_by_file(PyObject* dummy, PyObject* args) {
  const char* path;
  PyObject* all_as_plain;
  PyObject* ignore_bad_pattern;
  PyObject* bad_as_plain;
  PyObject* deduplicate_extra;
  matcher_t matcher = NULL;

  if (PyArg_ParseTuple(args, "sOOOO", &path, &all_as_plain, &ignore_bad_pattern, &bad_as_plain, &deduplicate_extra)) {
    matcher = matcher_construct_by_file(path, PyObject_IsTrue(all_as_plain), PyObject_IsTrue(ignore_bad_pattern),
                                        PyObject_IsTrue(bad_as_plain), PyObject_IsTrue(deduplicate_extra));
  }

  return Py_BuildValue("K", matcher);
}

PyObject* wrap_construct_by_string(PyObject* dummy, PyObject* args) {
  const char* string;
  Py_ssize_t length;
  PyObject* all_as_plain;
  PyObject* ignore_bad_pattern;
  PyObject* bad_as_plain;
  PyObject* deduplicate_extra;
  matcher_t matcher = NULL;

  if (PyArg_ParseTuple(args, "s#OOOO", &string, &length, &all_as_plain, &ignore_bad_pattern, &bad_as_plain,
                       &deduplicate_extra)) {
    strlen_s vocab = {.ptr = (char*)string, .len = (size_t)length};
    matcher = matcher_construct_by_string(&vocab, PyObject_IsTrue(all_as_plain), PyObject_IsTrue(ignore_bad_pattern),
                                          PyObject_IsTrue(bad_as_plain), PyObject_IsTrue(deduplicate_extra));
  }

  return Py_BuildValue("K", matcher);
}

PyObject* wrap_destroy(PyObject* dummy, PyObject* args) {
  unsigned long long temp;
  matcher_t matcher;

  if (!PyArg_ParseTuple(args, "K", &temp)) {
    fprintf(stderr, "%s:%d wrong args\n", __FUNCTION__, __LINE__);
    Py_RETURN_FALSE;
  }

  matcher = (matcher_t)temp;
  matcher_destruct(matcher);

  Py_RETURN_TRUE;
}

PyObject* wrap_alloc_context(PyObject* dummy, PyObject* args) {
  unsigned long long temp;
  matcher_t matcher;
  utf8ctx_t utf8ctx = NULL;

  if (PyArg_ParseTuple(args, "K", &temp)) {
    matcher = (matcher_t)temp;
    utf8ctx = utf8ctx_alloc_context(matcher);
  }

  return Py_BuildValue("K", utf8ctx);
}

PyObject* wrap_free_context(PyObject* dummy, PyObject* args) {
  unsigned long long temp;
  utf8ctx_t utf8ctx;

  if (!PyArg_ParseTuple(args, "K", &temp)) {
    fprintf(stderr, "%s:%d wrong args\n", __FUNCTION__, __LINE__);
    Py_RETURN_FALSE;
  }

  utf8ctx = (utf8ctx_t)temp;
  utf8ctx_free_context(utf8ctx);

  Py_RETURN_TRUE;
}

PyObject* wrap_reset_context(PyObject* dummy, PyObject* args) {
  unsigned long long temp;
  utf8ctx_t utf8ctx;
  char* content;
  Py_ssize_t length;
  PyObject* return_byte_pos;

  if (!PyArg_ParseTuple(args, "Ks#O", &temp, &content, &length, &return_byte_pos)) {
    fprintf(stderr, "%s:%d wrong args\n", __FUNCTION__, __LINE__);
    Py_RETURN_FALSE;
  }

  utf8ctx = (utf8ctx_t)temp;
  if (!utf8ctx_reset_context(utf8ctx, content, length, PyObject_IsTrue(return_byte_pos))) {
    Py_RETURN_FALSE;
  }

  Py_RETURN_TRUE;
}

static inline PyObject* build_matched_output(utf8ctx_t utf8ctx, word_t matched_word) {
  return Py_BuildValue("(s#,i,i,s#)", matched_word->keyword.ptr, (Py_ssize_t)matched_word->keyword.len,
                       matched_word->pos.so, matched_word->pos.eo, matched_word->extra.ptr,
                       (Py_ssize_t)matched_word->extra.len);
}

typedef word_t (*utf8ctx_next_f)(utf8ctx_t utf8ctx);

PyObject* wrap_next0(PyObject* dummy, PyObject* args, utf8ctx_next_f utf8ctx_next_func) {
  unsigned long long temp;
  utf8ctx_t utf8ctx;

  if (!PyArg_ParseTuple(args, "K", &temp)) {
    fprintf(stderr, "%s:%d wrong args\n", __FUNCTION__, __LINE__);
    Py_RETURN_NONE;
  }

  utf8ctx = (utf8ctx_t)temp;
  word_t matched_word = utf8ctx_next_func(utf8ctx);
  if (matched_word != NULL) {
    return build_matched_output(utf8ctx, matched_word);
  }

  Py_RETURN_NONE;
}

PyObject* wrap_find_all0(PyObject* dummy, PyObject* args, utf8ctx_next_f utf8ctx_next_func) {
  unsigned long long temp;
  matcher_t matcher;
  char* content;
  Py_ssize_t length;
  PyObject* return_byte_pos;

  if (!PyArg_ParseTuple(args, "Ks#O", &temp, &content, &length, &return_byte_pos)) {
    fprintf(stderr, "%s:%d wrong args\n", __FUNCTION__, __LINE__);
    Py_RETURN_NONE;
  }

  matcher = (matcher_t)temp;

  utf8ctx_t utf8ctx = utf8ctx_alloc_context(matcher);
  if (utf8ctx == NULL) {
    Py_RETURN_NONE;
  }

  if (!utf8ctx_reset_context(utf8ctx, content, length, PyObject_IsTrue(return_byte_pos))) {
    Py_RETURN_NONE;
  }

  PyObject* list = PyList_New(0);
  word_t matched_word = utf8ctx_next_func(utf8ctx);
  while (matched_word != NULL) {
    PyObject* item = build_matched_output(utf8ctx, matched_word);
    PyList_Append(list, item);
    Py_DECREF(item);
    matched_word = utf8ctx_next_func(utf8ctx);
  }

  utf8ctx_free_context(utf8ctx);

  return list;
}

PyObject* wrap_next(PyObject* dummy, PyObject* args) {
  return wrap_next0(dummy, args, utf8ctx_next);
}

PyObject* wrap_find_all(PyObject* dummy, PyObject* args) {
  return wrap_find_all0(dummy, args, utf8ctx_next);
}

PyObject* wrap_next_prefix(PyObject* dummy, PyObject* args) {
  return wrap_next0(dummy, args, utf8ctx_next_prefix);
}

PyObject* wrap_find_all_prefix(PyObject* dummy, PyObject* args) {
  return wrap_find_all0(dummy, args, utf8ctx_next_prefix);
}

static PyMethodDef wrapMethods[] = {
    {"ConstructByFile", wrap_construct_by_file, METH_VARARGS, "construct matcher by file"},
    {"ConstructByString", wrap_construct_by_string, METH_VARARGS, "construct matcher by string"},
    {"Destruct", wrap_destroy, METH_VARARGS, "destruct matcher"},
    {"AllocContext", wrap_alloc_context, METH_VARARGS, "alloc iterator context"},
    {"FreeContext", wrap_free_context, METH_VARARGS, "free iterator context"},
    {"ResetContext", wrap_reset_context, METH_VARARGS, "reset iterator context"},
    {"Next", wrap_next, METH_VARARGS, "iterator next"},
    {"FindAll", wrap_find_all, METH_VARARGS, "find all matched strings"},
    {"NextPrefix", wrap_next_prefix, METH_VARARGS, "iterator next prefix"},
    {"FindAllPrefix", wrap_find_all_prefix, METH_VARARGS, "find all matched prefix strings"},
    {NULL, NULL}};

#ifdef IS_PY3K
static PyModuleDef matchModule = {PyModuleDef_HEAD_INIT, "_actrie", NULL, -1, wrapMethods};

PyMODINIT_FUNC PyInit__actrie() {
#else
PyMODINIT_FUNC init_actrie() {
#endif

  PyObject* m;

#ifdef IS_PY3K
  m = PyModule_Create(&matchModule);
  return m;
#else
  m = Py_InitModule("_actrie", wrapMethods);
#endif
}
