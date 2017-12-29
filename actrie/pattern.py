# coding=utf-8

import sys
from enum import Enum
from abc import ABCMeta, abstractmethod


is_py3k = bool(sys.version_info[0] == 3)


def convert2unicode(str):
    if is_py3k or isinstance(str, unicode):
        return str
    return str.decode('utf-8')


class PatternType(Enum):
    Pure = 0
    Alternation = 1
    AntiAmbiguous = 2
    Distance = 3


class Pattern:
    __metaclass__ = ABCMeta

    def __init__(self):
        self.type = None

    @staticmethod
    def parse(pattern):
        pattern = convert2unicode(pattern)
        state = 0
        for ch in pattern:
            pass

    def __str__(self):
        return u''


class PurePattern(Pattern):

    def __init__(self, text):
        Pattern.__init__(self)
        self.type = PatternType.Pure
        self.text = text

    def __str__(self):
        return self.text


class AlternationPattern(Pattern):

    def __init__(self, patterns):
        Pattern.__init__(self)
        self.type = PatternType.Alternation
        self.patterns = set(patterns)

    def __str__(self):
        return u"(" + u"|".join([str(p) for p in self.patterns]) + u")"


class AntiAmbiguousPattern(Pattern):

    def __init__(self, origin, ambis):
        Pattern.__init__(self)
        self.type = PatternType.AntiAmbiguous
        self.origin = origin
        self.ambis = set(ambis)

    def __str__(self):
        return str(self.origin) + u"(?&!" + u"|".join([str(p) for p in self.ambis]) + u")"


class DistancePattern(Pattern):

    def __init__(self, head, tail, dist):
        Pattern.__init__(self)
        self.type = PatternType.Distance
        self.head = head
        self.tail = tail
        self.dist = dist

    def __str__(self):
        return str(self.head) + u".{0," + convert2unicode(str(self.dist)) + u"}" + str(self.tail)


if __name__ == "__main__":
    a = PurePattern("aaa")
    b = PurePattern("bbb")
    ab = PurePattern("ab")
    ba = PurePattern("ba")
    ambi = AntiAmbiguousPattern(a, [ab, ba])
    alter = AlternationPattern([ambi, b])
    c = PurePattern("ccc")
    dist = DistancePattern(alter, c, 12)
    print(dist)