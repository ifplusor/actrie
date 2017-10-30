#!/usr/bin/env python

import actrie

pattern = "(f|(a|b).{0,5}(e(?&!ef)|g))"
content = "abcdefg"

if __name__ == "__main__":
    matcher = actrie.Matcher()
    matcher.load_from_string(pattern)

    print "\ntest context"
    context = matcher.match(content)
    for matched in context:
        print matched

    print "\ntest not reset"
    for matched in context:
        print matched

    print "\ntest reset"
    context.reset()
    for matched in context:
        print matched

    print "\ntest find all"
    ret = matcher.find_all(content)
    print(ret)
