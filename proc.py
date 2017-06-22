#!/usr/bin/env python2
# -*- coding=UTF-8 -*-

import re

pattern = re.compile("(.*)(?:\.\*\?|\?\((\d+)\)\+{5})(.*)")

fout1 = open('t1.txt', 'w')
fout2 = open('t2.txt', 'w')

count = 1

with open('js.txt', 'r') as fin:
    for line in fin:
        for word in line.split(' '):
            word = word.strip()
            if word == "":
                continue

            r = pattern.match(word)
            if r:
                head, distance, tail = r.groups()

                if not head or not tail:
                    continue

                # fout1.write("%s\n" % word)
                # continue

                if not distance:
                    distance = "15"

                for h in head.split(","):
                    for t in tail.split(","):
                        fout1.write(
                            "%s\t%d###%c\n" % (head, count, 48 + int(distance)*3))
                        fout2.write("%s\t%d\n" % (tail, count))
                        count += 1

fout1.close()
fout2.close()
