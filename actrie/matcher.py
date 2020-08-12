# coding=utf-8

import os

from . import _actrie
from .context import Context, PrefixContext
from .error import MatcherError
from .util import convert2pass


class Matcher:
    def __init__(self):
        self._matcher = 0

    def __del__(self):
        _actrie.Destruct(self._matcher)

    def load_from_file(
        self, path, all_as_plain=False, ignore_bad_pattern=False, bad_as_plain=True, deduplicate_extra=True
    ):
        if self._matcher:
            raise MatcherError("Matcher is already initialized.")
        if not os.path.isfile(path):
            return False
        self._matcher = _actrie.ConstructByFile(
            convert2pass(path), all_as_plain, ignore_bad_pattern, bad_as_plain, deduplicate_extra
        )
        return self._matcher != 0

    @classmethod
    def create_by_file(
        cls, path, all_as_plain=False, ignore_bad_pattern=False, bad_as_plain=True, deduplicate_extra=True
    ):
        matcher = cls()
        if matcher.load_from_file(path, all_as_plain, ignore_bad_pattern, bad_as_plain, deduplicate_extra):
            return matcher
        return None

    def load_from_string(
        self, keywords, all_as_plain=False, ignore_bad_pattern=False, bad_as_plain=True, deduplicate_extra=True
    ):
        if self._matcher:
            raise MatcherError("Matcher is already initialized.")
        if keywords is None:
            return False
        self._matcher = _actrie.ConstructByString(
            convert2pass(keywords), all_as_plain, ignore_bad_pattern, bad_as_plain, deduplicate_extra
        )
        return self._matcher != 0

    @classmethod
    def create_by_string(
        cls, keywords, all_as_plain=False, ignore_bad_pattern=False, bad_as_plain=True, deduplicate_extra=True
    ):
        matcher = cls()
        if matcher.load_from_string(keywords, all_as_plain, ignore_bad_pattern, bad_as_plain, deduplicate_extra):
            return matcher
        return None

    def load_from_collection(
        self, keywords, all_as_plain=False, ignore_bad_pattern=False, bad_as_plain=True, deduplicate_extra=True
    ):
        if self._matcher:
            raise MatcherError("Matcher is already initialized.")
        if isinstance(keywords, list) or isinstance(keywords, set):
            # for utf-8 '\n' is 0x0a, in other words, utf-8 is ascii compatible.
            # but in python3, str.join is only accept str as argument
            keywords = "\n".join([convert2pass(keyword) for keyword in keywords if convert2pass(keyword)])
        else:
            raise MatcherError("Keywords should be list or set.")
        return self.load_from_string(keywords, all_as_plain, ignore_bad_pattern, bad_as_plain, deduplicate_extra)

    @classmethod
    def create_by_collection(
        cls, keywords, all_as_plain=False, ignore_bad_pattern=False, bad_as_plain=True, deduplicate_extra=True
    ):
        matcher = cls()
        if matcher.load_from_collection(keywords, all_as_plain, ignore_bad_pattern, bad_as_plain, deduplicate_extra):
            return matcher
        return None

    def match(self, content, return_byte_pos=False):
        if not self._matcher:
            raise MatcherError("Matcher is not initialized.")
        return Context(self, content, return_byte_pos)

    def findall(self, content, return_byte_pos=False):
        """Return a list of all matches of pattern in string.

        :type content: str
        :rtype: list[(str, int, int, str)]
        """
        if not self._matcher:
            raise MatcherError("Matcher is not initialized.")
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
            raise MatcherError("Matcher is not initialized.")
        return PrefixContext(self, content, return_byte_pos)

    def findall(self, content, return_byte_pos=False):
        """Return a list of all matches of pattern in string.

        :type content: str
        :rtype: list[(str, int, int, str)]
        """
        if not self._matcher:
            raise MatcherError("Matcher is not initialized.")
        return _actrie.FindAllPrefix(self._matcher, convert2pass(content), return_byte_pos)
