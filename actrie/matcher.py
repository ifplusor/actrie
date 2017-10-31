# coding=utf-8

import os

from . import _actrie
from .context import Context


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
        # if isinstance(keyword, unicode):
        #     keyword = keyword.encode("utf-8")
        self._matcher = _actrie.ConstructByString(keyword)
        return self._matcher is not None

    def load_from_collection(self, strings):
        if self._matcher:
            return MatcherError("matcher is initialized")
        if isinstance(strings, list) or isinstance(strings, set):
            keywords = "\n".join(
                [word.encode("utf8") if isinstance(word, unicode) else word
                 for word in strings
                 if isinstance(word, unicode) or isinstance(word, str)])
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
