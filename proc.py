#!/usr/bin/env python2
# -*- coding=UTF-8 -*-

fout1 = open('t1.txt', 'w')
fout2 = open('t2.txt', 'w')

count = 1
with open('js.txt', 'r') as fin:
    for line in fin:
        for word in line.split(' '):
            if len(word) > 0 and word != ' ':
                if word.find(".*?") != -1:
                    w = word.split(".*?")
                    if len(w) != 2:
                        continue;
                    fout1.write("%s\t%d###%c\n" % (w[0].strip(), count, 48+15))
                    fout2.write("%s\t%d\n" % (w[1].strip(), count))
                    count+=1
                
fout1.close()
fout2.close()

