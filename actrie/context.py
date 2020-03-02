# coding=utf-8

from collections import Iterator

from . import _actrie


class Context(Iterator):
    def __init__(self, matcher, content):
        self._matcher = matcher
        self._content = content
        self._context = _actrie.AllocContext(self._matcher._matcher)
        _actrie.ResetContext(self._context, self._content)

    def __del__(self):
        _actrie.FreeContext(self._context)

    def reset(self):
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
