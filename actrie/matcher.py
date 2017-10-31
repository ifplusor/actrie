# coding=utf-8

import os
import sys

from . import _actrie
from .context import Context

is_py3k = bool(sys.version_info[0] == 3)


def convert2pass(obj):
    # we only convert unicode in python2 to utf-8
    if not is_py3k and isinstance(obj, unicode):
        obj = obj.encode("utf8")
    return obj


class MatcherError(Exception):
    def __init__(self, value):
        self.value = value

    def __str__(self):
        return repr(self.value)


class Matcher:
    def __init__(self):
        self._matcher = None

    def __del__(self):
        if self._matcher:
            _actrie.Destruct(self._matcher)

    def load_from_file(self, path):
        if self._matcher:
            return MatcherError("matcher is initialized")
        if not os.path.isfile(path):
            return False
        self._matcher = _actrie.ConstructByFile(path)
        return self._matcher is not None

    def load_from_string(self, keyword):
        if self._matcher:
            return MatcherError("matcher is initialized")
        if not keyword:
            return False
        keyword = convert2pass(keyword)
        self._matcher = _actrie.ConstructByString(keyword)
        return self._matcher is not None

    def load_from_collection(self, strings):
        if self._matcher:
            return MatcherError("matcher is initialized")
        if isinstance(strings, list) or isinstance(strings, set):
            # for utf-8 '\n' is 0x0a, in other words, utf-8 is ascii compatible.
            # but in python3, str.join is only accept str as argument
            keywords = "\n".join(
                [convert2pass(word) for word in strings
                 if convert2pass(word) is not None])
        else:
            raise MatcherError("should be list or set")
        return self.load_from_string(keywords)

    def find_all(self, content):
        if not self._matcher:
            return MatcherError("matcher is not initialized")
        return _actrie.FindAll(self._matcher, content)

    def match(self, content):
        if not self._matcher:
            raise MatcherError("matcher is not initialized")
        return Context(self, content)
