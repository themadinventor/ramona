#!/usr/bin/python
#
# Patching Tool
#
# (c) 2012 <fredrik@z80.se>

class Symlist:

    def __init__(self, filename):
        self.syms = {}
        f = file(filename)
        while True:
            s = f.readline().rstrip()
            if len(s) < 1:
                break
            s = s.split()
            if len(s) < 3:
                break
            self.syms[s[2]] = int(s[0], 16)
