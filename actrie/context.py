from collections import Iterator

import match


class Context(Iterator):
    def __init__(self, matcher, content):
        self._matcher = matcher
        self._content = content
        self._context = match.AllocContext(self._matcher._matcher)
        match.ResetContext(self._context, self._content)

    def __del__(self):
        match.FreeContext(self._context)

    def reset(self):
        match.ResetContext(self._context, self._content)

    def next(self):
        matched = match.Next(self._context)
        if matched is None:
            raise StopIteration
        return matched
