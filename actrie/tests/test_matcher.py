#!/usr/bin/env python
# coding=utf-8

from actrie import *

pattern = "中国\n北京"
content = "中国，北京"


def test():
    matcher = Matcher.create_by_string(pattern)

    print("\ntest context:")
    context = matcher.match(content)
    for matched in context:
        print("{}: so:{}, eo:{}, extra:{}".format(*matched))

    print("\ntest not reset:")
    for matched in context:
        print("{}: so:{}, eo:{}, extra:{}".format(*matched))

    print("\ntest reset:")
    context.reset(return_byte_pos=True)
    for matched in context:
        print("{}: so:{}, eo:{}, extra:{}".format(*matched))

    print("\ntest find all:")
    matched_list = matcher.findall(content)
    for matched in matched_list:
        print("{}: so:{}, eo:{}, extra:{}".format(*matched))


if __name__ == "__main__":
    test()
