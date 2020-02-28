# encoding=utf-8

import sys

is_py3k = bool(sys.version_info[0] == 3)


def convert2pass(obj):
    # we only convert unicode in python2 to utf-8
    if not is_py3k and isinstance(obj, unicode):
        obj = obj.encode("utf8")
    return obj


def convert2unicode(str):
    if is_py3k or isinstance(str, unicode):
        return str
    return str.decode('utf-8')
