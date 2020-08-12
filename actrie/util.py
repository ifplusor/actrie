# encoding=utf-8

import sys

try:
    from typing import AnyStr
except Exception:
    pass

is_py2k = bool(sys.version_info[0] == 2)

if not is_py2k:

    def convert2pass(string):
        # type: (AnyStr) -> str
        if isinstance(string, bytes):
            return string.decode("utf-8")
        return string

    def convert2unicode(string):
        # type: (AnyStr) -> str
        if isinstance(string, bytes):
            return string.decode("utf-8")
        return string

    def replace_escap(word):
        for old, new in (
            (u"\\", u"\\\\"),
            (u"(", u"\\("),
            (u")", u"\\)"),
            (u"{", u"\\{"),
            (u".", u"\\."),
            (u"|", u"\\|"),
        ):
            word = word.replace(old, new)
        return word


else:

    def convert2pass(string):
        # type: (AnyStr) -> str
        if isinstance(string, unicode):
            return string.encode("utf-8")
        return string

    def convert2unicode(string):
        # type: (AnyStr) -> unicode
        if isinstance(string, str):
            return string.decode("utf-8")
        return string

    def replace_escap(word):
        if isinstance(word, unicode):
            for old, new in (
                (u"\\", u"\\\\"),
                (u"(", u"\\("),
                (u")", u"\\)"),
                (u"{", u"\\{"),
                (u".", u"\\."),
                (u"|", u"\\|"),
            ):
                word = word.replace(old, new)
        else:
            for old, new in (("\\", "\\\\"), ("(", "\\("), (")", "\\)"), ("{", "\\{"), (".", "\\."), ("|", "\\|")):
                word = word.replace(old, new)
        return word
