# coding=utf-8

from collections import Iterator

from . import _actrie
from .util import convert2pass


class Context(Iterator):
    def __init__(self, matcher, content):
        self._matcher = matcher
        self._content = convert2pass(content)
        self._context = _actrie.AllocContext(self._matcher._matcher)
        _actrie.ResetContext(self._context, self._content)

    def __del__(self):
        _actrie.FreeContext(self._context)

    def reset(self, content=None):
        if content is not None:
            self._content = convert2pass(content)
        _actrie.ResetContext(self._context, self._content)

    def __next__(self):
        matched = _actrie.Next(self._context)
        if matched is None:
            raise StopIteration
        return matched

    def next(self):
        return self.__next__()


class PrefixContext(Context):
    def __next__(self):
        matched = _actrie.NextPrefix(self._context)
        if matched is None:
            raise StopIteration
        return matched
