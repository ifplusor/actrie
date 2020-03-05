# encoding=utf-8

import sys

is_py3k = bool(sys.version_info[0] == 3)


def convert2pass(obj):
    if is_py3k:
        if isinstance(obj, bytes):
            return obj.decode("utf-8")
    else:
        if isinstance(obj, unicode):
            return obj.encode("utf-8")
    return obj


def convert2unicode(obj):
    if is_py3k:
        if isinstance(obj, bytes):
            return obj.decode("utf-8")
    else:
        if isinstance(obj, str):
            return obj.decode("utf-8")
    return obj


def replace_escap(word):
    if is_py3k or isinstance(str, unicode):
        for old, new in ((u"\\", u"\\\\"), (u"(", u"\\("), (u")", u"\\)"), (u"{", u"\\{"), (u".", u"\\."), (u"|", u"\\|")):
            word = word.replace(old, new)
    else:
        for old, new in (("\\", "\\\\"), ("(", "\\("), (")", "\\)"), ("{", "\\{"), (".", "\\."), ("|", "\\|")):
            word = word.replace(old, new)
    return word
