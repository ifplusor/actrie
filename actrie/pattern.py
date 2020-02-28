# coding=utf-8

from abc import ABCMeta, abstractmethod

try:
    from enum import Enum
except:
    class Enum(object):
        pass

from .tokenizer import Tokenizer
from .util import convert2unicode, is_py3k

meta_char = {u"(", u")", u"|", u"."}


class PatternError(Exception):
    def __init__(self, value):
        self.value = value

    def __str__(self):
        return repr(self.value)


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
        if pattern == u"":
            return None

        tokenizer = Tokenizer(u"(" + pattern + u")")

        sign_stack = []
        data_stack = []
        while 1:
            s, ch = tokenizer.consume(meta_char)

            # 纯文本
            if s != u"":
                data_stack.append(PurePattern(s))

            if ch == u"(":
                if tokenizer.expect(u"?&!"):
                    # 歧义
                    sign_stack.append(u"(?&!")
                else:
                    # 括号前不允许纯文本
                    if s != u"":
                        raise PatternError("expression error")
                    sign_stack.append(u"(")
            elif ch == u".":
                if not tokenizer.expect(u"{"):
                    raise PatternError("parse error")
                dmin = tokenizer.consume_integer()
                if dmin is None:
                    raise PatternError("parse error")
                if not tokenizer.expect(u","):
                    raise PatternError("parse error")
                dmax = tokenizer.consume_integer()
                if dmax is None:
                    raise PatternError("parse error")
                if dmax < dmin:
                    raise PatternError("expression error")
                if not tokenizer.expect(u"}"):
                    raise PatternError("parse error")
                # 距离
                sign_stack.append(u".{}")
                data_stack.append((dmin, dmax))
            elif ch in {u"|", u")"}:
                if sign_stack[-1] == u".{}":
                    # 规约距离表达式
                    sign_stack.pop()
                    tail = data_stack.pop()
                    dist = data_stack.pop()
                    head = data_stack.pop()
                    data_stack.append(DistancePattern(head, tail, dist[1]))
                if ch == u")":
                    # 规约“并列”或“歧义”
                    alters = set()
                    alters.add(data_stack.pop())
                    sign0 = sign_stack.pop()
                    while sign0 == u"|":
                        alters.add(data_stack.pop())
                        sign0 = sign_stack.pop()
                    if len(alters) == 0:
                        raise PatternError("expression error")
                    if sign0 == u"(":
                        if len(alters) == 1:
                            data_stack.append(list(alters)[0])
                        else:
                            data_stack.append(AlternationPattern(alters))
                    elif sign0 == u"(?&!":
                        origin = data_stack.pop()
                        data_stack.append(AntiAmbiguousPattern(origin, alters))
                    else:
                        raise PatternError("expression error")
                else:
                    sign_stack.append(u"|")
            elif ch == -1:
                break
            else:
                raise PatternError("parse error")

        if len(sign_stack):
            raise PatternError("expression error")

        return data_stack[0]

    @staticmethod
    def _parse_alter(tokenizer, stop=None):

        alters = list()

        while True:

            s, ch = tokenizer.consume(meta_char)

            if ch == u"|":
                p = PurePattern(s)
                alters.append(p)
            elif (stop and ch == stop) or stop == -1:
                break

        if len(alters) == 1:
            return alters[0]
        else:
            return AlternationPattern(alters)

    @staticmethod
    def _parse_all(tokenizer, stop=None):

        if tokenizer.expect(u"("):
            return Pattern._parse_alter(tokenizer, u")")

        return None, ""

    @abstractmethod
    def ustr(self):
        return u''


class PurePattern(Pattern):
    def __init__(self, text):
        Pattern.__init__(self)
        self.type = PatternType.Pure
        self.text = text

    def __getitem__(self, i):
        return self.text[i]

    def ustr(self):
        return self.text

    def __str__(self):
        return self.ustr() if is_py3k else self.ustr().encode('utf-8')


class AlternationPattern(Pattern):
    def __init__(self, alters):
        Pattern.__init__(self)
        self.type = PatternType.Alternation
        self.alters = set(alters)

    def ustr(self):
        if len(self.alters) == 1:
            return list(self.alters)[0].ustr()
        else:
            return u"(" + u"|".join([p.ustr() for p in self.alters]) + u")"

    def __str__(self):
        return self.ustr() if is_py3k else self.ustr().encode('utf-8')


class AntiAmbiguousPattern(Pattern):
    def __init__(self, origin, ambis):
        Pattern.__init__(self)
        self.type = PatternType.AntiAmbiguous
        self.origin = origin
        self.ambis = set(ambis)

    def ustr(self):
        return self.origin.ustr() + u"(?&!" + u"|".join([p.ustr() for p in self.ambis]) + u")"

    def __str__(self):
        return self.ustr() if is_py3k else self.ustr().encode('utf-8')


class DistancePattern(Pattern):
    def __init__(self, head, tail, dist):
        Pattern.__init__(self)
        self.type = PatternType.Distance
        self.head = head
        self.tail = tail
        self.dist = dist

    def ustr(self):
        return self.head.ustr() + u".{0," + convert2unicode(str(self.dist)) + u"}" + self.tail.ustr()

    def __str__(self):
        return self.ustr() if is_py3k else self.ustr().encode('utf-8')
