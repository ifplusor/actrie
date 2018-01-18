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

    @classmethod
    def create_by_file(cls, path):
        matcher = Matcher()
        if matcher.load_from_file(path):
            return matcher
        return None

    def load_from_string(self, keyword):
        if self._matcher:
            return MatcherError("matcher is initialized")
        if not keyword:
            return False
        keyword = convert2pass(keyword)
        self._matcher = _actrie.ConstructByString(keyword)
        return self._matcher is not None

    @classmethod
    def create_by_string(cls, keyword):
        matcher = Matcher()
        if matcher.load_from_string(keyword):
            return matcher
        return None

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

    @classmethod
    def create_by_collection(cls, strings):
        matcher = Matcher()
        if matcher.load_from_collection(strings):
            return matcher
        return None

    def match(self, content):
        if not self._matcher:
            raise MatcherError("matcher is not initialized")
        return Context(self, content)

    def findall(self, content):
        """Return a list of all matches of pattern in string.

        :type content: str
        :rtype: list[(str, int, int, str)]
        """
        if not self._matcher:
            return MatcherError("matcher is not initialized")
        return _actrie.FindAll(self._matcher, content)

    def finditer(self, content):
        return self.match(content)

    def search(self, content):
        """Return first matched.

        :type content: str
        :rtype: (str, int, int, str)
        """
        ctx = self.finditer(content)
        for matched in ctx:
            return matched
        return None


class ExternalMatcher:
    def __init__(self):
        pass
