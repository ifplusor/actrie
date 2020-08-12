# encoding=utf-8

from collections import Iterator

from . import _actrie
from .error import MatcherError
from .util import convert2pass


class Context(Iterator):
    def __init__(self, matcher, content=None, return_byte_pos=False):
        if matcher is None:
            raise MatcherError("Matcher is None")

        self._matcher = matcher
        self._return_byte_pos = return_byte_pos

        if content is None:
            self._content = None
            self._context = 0
            self._uninitialized = False
        else:
            self._context = _actrie.AllocContext(self._matcher._matcher)
            self.reset(content, return_byte_pos)

    def __del__(self):
        _actrie.FreeContext(self._context)

    def reset(self, content=None, return_byte_pos=None):
        if content is not None:
            self._content = convert2pass(content)
        if return_byte_pos is not None:
            self._return_byte_pos = return_byte_pos
        self._uninitialized = not _actrie.ResetContext(self._context, self._content, self._return_byte_pos)
        if self._uninitialized:
            raise MatcherError("Reset context failed!")

    def _next(self):
        if self._uninitialized:
            return None
        return _actrie.Next(self._context)

    def __next__(self):
        # python3
        matched = self._next()
        if matched is None:
            raise StopIteration()
        return matched

    def next(self):
        # python2
        return self.__next__()


class PrefixContext(Context):
    def _next(self):
        if self._uninitialized:
            raise None
        return _actrie.NextPrefix(self._context)
