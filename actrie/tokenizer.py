# coding=utf-8

from .util import convert2unicode


class TokenizerError(Exception):
    def __init__(self, value):
        self.value = value

    def __str__(self):
        return repr(self.value)


number_char = [convert2unicode(str(x)) for x in range(10)]


class Tokenizer:

    def __init__(self, text):
        self.text = text
        self.cur = 0

    @staticmethod
    def escape_char(ch):
        if ch == u"\\":
            return u"\\"
        elif ch == u"t":
            return u"\t"
        elif ch == u"r":
            return u"\r"
        elif ch == u"n":
            return u"\n"
        elif ch == u"(":
            return u"("
        elif ch == u")":
            return u")"

        raise TokenizerError("'\\" + ch + "' is not a escape character")

    def consume(self, stop=set()):
        s = u""
        is_escape = False
        so = self.cur

        while self.cur < len(self.text):
            ch = self.text[self.cur]
            self.cur += 1

            if is_escape:
                ch = self.escape_char(ch)
                is_escape = False
            else:
                if ch == u"\\":
                    is_escape = True
                    continue

                if ch in stop and (ch != u"." or self.cur >= len(self.text) or self.text[self.cur] == "{"):
                    return s, ch

            s += ch

        return s, -1

    def consume_integer(self):
        s = u""
        while self.cur < len(self.text):
            ch = self.text[self.cur]
            if ch not in number_char:
                break
            s += ch
            self.cur += 1
        return int(s) if s else None

    def expect(self, s, move=True):
        wlen = len(s)
        if len(self.text) - self.cur >= wlen and self.text[self.cur:(self.cur + wlen)] == s:
            if move:
                self.cur += wlen
            return True
        return False

    def eof(self):
        return self.cur == len(self.text)
