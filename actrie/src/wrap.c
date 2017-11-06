/**
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

#include <Python.h>
#include <matcher.h>

#if PY_MAJOR_VERSION >= 3
#define IS_PY3K
#endif

PyObject *wrap_construct_by_file(PyObject *dummy, PyObject *args) {
  const char *path;
  matcher_t matcher;

  if (!PyArg_ParseTuple(args, "s", &path)) {
    fprintf(stderr, "%s:%d wrong args\n", __FUNCTION__, __LINE__);
    return Py_None;
  }

  matcher = matcher_construct_by_file(matcher_type_ultimate, path);
  if (matcher == NULL)
    return Py_None;

  return Py_BuildValue("K", matcher);
}

PyObject *wrap_construct_by_string(PyObject *dummy, PyObject *args) {
  const char *string;
  int length;
  matcher_t matcher;

  if (!PyArg_ParseTuple(args, "s#", &string, &length)) {
    fprintf(stderr, "%s:%d wrong args\n", __FUNCTION__, __LINE__);
    return Py_None;
  }

  strlen_s vocab = {
      .ptr = (char *) string,
      .len = (size_t) length
  };
  matcher = matcher_construct_by_string(matcher_type_ultimate, &vocab);
  if (matcher == NULL)
    return Py_None;

  return Py_BuildValue("K", matcher);
}

PyObject *wrap_destroy(PyObject *dummy, PyObject *args) {
  unsigned long long temp;
  matcher_t matcher;

  if (!PyArg_ParseTuple(args, "K", &temp)) {
    fprintf(stderr, "%s:%d wrong args\n", __FUNCTION__, __LINE__);
    return Py_None;
  }

  matcher = (matcher_t) temp;

  if (matcher_destruct(matcher))
    return Py_True;

  return Py_False;
}

PyObject *wrap_alloc_context(PyObject *dummy, PyObject *args) {
  unsigned long long temp;
  matcher_t matcher;
  context_t context;

  if (!PyArg_ParseTuple(args, "K", &temp)) {
    fprintf(stderr, "%s:%d wrong args\n", __FUNCTION__, __LINE__);
    return Py_None;
  }

  matcher = (matcher_t) temp;

  context = matcher_alloc_context(matcher);
  if (context == NULL)
    return Py_None;

  return Py_BuildValue("K", context);
}

PyObject *wrap_free_context(PyObject *dummy, PyObject *args) {
  unsigned long long temp;
  context_t context;

  if (!PyArg_ParseTuple(args, "K", &temp)) {
    fprintf(stderr, "%s:%d wrong args\n", __FUNCTION__, __LINE__);
    return Py_None;
  }

  context = (context_t) temp;

  if (matcher_free_context(context))
    return Py_True;

  return Py_False;
}

PyObject *wrap_reset_context(PyObject *dummy, PyObject *args) {
  unsigned long long temp;
  context_t context;
  char *content;
  int length;

  if (!PyArg_ParseTuple(args, "Ks#", &temp, &content, &length)) {
    fprintf(stderr, "%s:%d wrong args\n", __FUNCTION__, __LINE__);
    return Py_None;
  }

  context = (context_t) temp;

  if (matcher_reset_context(context, content, (size_t) length))
    return Py_True;

  return Py_False;
}

static inline PyObject *build_matched_output(mdi_t mdi, strpos_s pos) {
  return Py_BuildValue(
      "(s#,i,i,s)", mdi->keyword, mdi->length, pos.so, pos.eo, mdi->extra);
}

PyObject *wrap_next(PyObject *dummy, PyObject *args) {
  unsigned long long temp;
  context_t context;

  if (!PyArg_ParseTuple(args, "K", &temp)) {
    fprintf(stderr, "%s:%d wrong args\n", __FUNCTION__, __LINE__);
    return Py_None;
  }

  context = (context_t) temp;

  if (matcher_next(context)) {
    mdi_t mdi = matcher_matched_index(context);
    strpos_s pos = matcher_matched_pos(context);
    return build_matched_output(mdi, pos);
  }

  return Py_None;
}

PyObject *wrap_find_all(PyObject *dummy, PyObject *args) {
  unsigned long long temp;
  matcher_t matcher;
  char *content;
  int length;

  if (!PyArg_ParseTuple(args, "Ks#", &temp, &content, &length)) {
    fprintf(stderr, "%s:%d wrong args\n", __FUNCTION__, __LINE__);
    return Py_None;
  }

  matcher = (matcher_t) temp;

  context_t context = matcher_alloc_context(matcher);
  if (context == NULL)
    return Py_None;

  if (!matcher_reset_context(context, content, (size_t) length))
    return Py_None;

  PyObject *list = PyList_New(0);
  while (matcher_next(context)) {
    mdi_t mdi = matcher_matched_index(context);
    strpos_s pos = matcher_matched_pos(context);
    PyObject *item = build_matched_output(mdi, pos);
    PyList_Append(list, item);
  }

  return list;
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
    {NULL, NULL}
};

#ifdef IS_PY3K
static PyModuleDef matchModule = {
    PyModuleDef_HEAD_INIT, "_actrie", NULL, -1, wrapMethods
};

PyMODINIT_FUNC PyInit__actrie() {
#else
PyMODINIT_FUNC init_actrie() {
#endif

  PyObject *m;

#ifdef IS_PY3K
  m = PyModule_Create(&matchModule);
  return m;
#else
  m = Py_InitModule("_actrie", wrapMethods);
#endif
}
