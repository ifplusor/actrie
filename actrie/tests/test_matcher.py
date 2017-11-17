#!/usr/bin/env python
# coding=utf-8

from actrie import *

pattern = "(f|(a|b).{0,5}(e(?&!ef)|g))"
content = "abcdefg"


def test():
    matcher = Matcher()
    matcher.load_from_string(pattern)

    print("\ntest context")
    context = matcher.match(content)
    for matched in context:
        print(matched)

    print("\ntest not reset")
    for matched in context:
        print(matched)

    print("\ntest reset")
    context.reset()
    for matched in context:
        print(matched)

    print("\ntest find all")
    ret = matcher.findall(content)
    print(ret)


if __name__ == "__main__":
    test()
