# coding=utf-8

import os

from . import _actrie
from .context import Context, PrefixContext
from .util import convert2pass


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

    def load_from_file(self, path, all_as_plain=False, ignore_bad_pattern=False, bad_as_plain=True):
        if self._matcher:
            raise MatcherError("matcher is initialized")
        if not os.path.isfile(path):
            return False
        self._matcher = _actrie.ConstructByFile(
            convert2pass(path), all_as_plain, ignore_bad_pattern, bad_as_plain)
        return self._matcher is not None

    @classmethod
    def create_by_file(cls, path, all_as_plain=False, ignore_bad_pattern=False, bad_as_plain=True):
        matcher = cls()
        if matcher.load_from_file(path, all_as_plain, ignore_bad_pattern, bad_as_plain):
            return matcher
        return None

    def load_from_string(self, keyword, all_as_plain=False, ignore_bad_pattern=False, bad_as_plain=True):
        if self._matcher:
            raise MatcherError("matcher is initialized")
        if keyword is None:
            return False
        self._matcher = _actrie.ConstructByString(
            convert2pass(keyword), all_as_plain, ignore_bad_pattern, bad_as_plain)
        return self._matcher is not None

    @classmethod
    def create_by_string(cls, keyword, all_as_plain=False, ignore_bad_pattern=False, bad_as_plain=True):
        matcher = cls()
        if matcher.load_from_string(keyword, all_as_plain, ignore_bad_pattern, bad_as_plain):
            return matcher
        return None

    def load_from_collection(self, strings, all_as_plain=False, ignore_bad_pattern=False, bad_as_plain=True):
        if self._matcher:
            raise MatcherError("matcher is initialized")
        if isinstance(strings, list) or isinstance(strings, set):
            # for utf-8 '\n' is 0x0a, in other words, utf-8 is ascii compatible.
            # but in python3, str.join is only accept str as argument
            keywords = "\n".join(
                [convert2pass(word) for word in strings
                 if convert2pass(word) is not None])
        else:
            raise MatcherError("should be list or set")
        return self.load_from_string(keywords, all_as_plain, ignore_bad_pattern, bad_as_plain)

    @classmethod
    def create_by_collection(cls, strings, all_as_plain=False, ignore_bad_pattern=False, bad_as_plain=True):
        matcher = cls()
        if matcher.load_from_collection(strings, all_as_plain, ignore_bad_pattern, bad_as_plain):
            return matcher
        return None

    def match(self, content, return_byte_pos=False):
        if not self._matcher:
            raise MatcherError("matcher is not initialized")
        return Context(self, content, return_byte_pos)

    def findall(self, content, return_byte_pos=False):
        """Return a list of all matches of pattern in string.

        :type content: str
        :rtype: list[(str, int, int, str)]
        """
        if not self._matcher:
            raise MatcherError("matcher is not initialized")
        return _actrie.FindAll(self._matcher, convert2pass(content), return_byte_pos)

    def finditer(self, content, return_byte_pos=False):
        return self.match(content, return_byte_pos)

    def search(self, content, return_byte_pos=False):
        """Return first matched.

        :type content: str
        :rtype: (str, int, int, str)
        """
        ctx = self.finditer(content, return_byte_pos)
        for matched in ctx:
            return matched
        return None


class PrefixMatcher(Matcher):
    def match(self, content, return_byte_pos=False):
        if not self._matcher:
            raise MatcherError("matcher is not initialized")
        return PrefixContext(self, content, return_byte_pos)

    def findall(self, content, return_byte_pos=False):
        """Return a list of all matches of pattern in string.

        :type content: str
        :rtype: list[(str, int, int, str)]
        """
        if not self._matcher:
            raise MatcherError("matcher is not initialized")
        return _actrie.FindAllPrefix(self._matcher, convert2pass(content), return_byte_pos)
