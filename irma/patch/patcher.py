#!/usr/bin/python
#
# Binary Patching Toolkit
#
# (c) 2012 <fredrik@z80.se>

import struct
import hashlib

class Patcher:

    def __init__(self, filename = None, addr = None):
        self._bin = None
        self._base = 0

        if filename is not None:
            self.load(filename)

        if addr is not None:
            self.translate(addr)

    def load(self, filename):
        self._bin = file(filename).read()

    def save(self, filename):
        file(filename, 'w').write(self._bin)

    def translate(self, addr):
        self._base = addr

    @property
    def hash(self):
        m = hashlib.md5()
        m.update(self._bin)
        return m.hexdigest()

    @property
    def blobhash(self):
        m = hashlib.md5()
        m.update(self._bin[8:])
        d = m.digest()[:4]
        return d[3]+d[2]+d[1]+d[0]

    @property
    def size(self):
        return len(self._bin)

    def get(self, addr, len, fmt):
        addr -= self._base
        s = self._bin[addr:addr+len]
        return struct.unpack(fmt, s)

    def set(self, addr, len, fmt, value):
        addr -= self._base
        s = struct.pack(fmt, value)
        self._bin = self._bin[:addr] + s + self._bin[addr+len:]

    def getb(self, addr):
        return self.get(addr, 1, 'B')[0]

    def gets(self, addr):
        return self.get(addr, 2, '<H')[0]

    def getl(self, addr):
        return self.get(addr, 4, '<I')[0]

    def gets(self, addr):
        s = ''
        while True:
            c = self.getb(addr)
            if c == 0:
                return s
            else:
                s = s + chr(c)
                addr += 1

    def setb(self, addr, val):
        self.set(addr, 1, 'B', val)

    def setw(self, addr, val):
        self.set(addr, 2, '<H', val)

    def setl(self, addr, val):
        self.set(addr, 4, '<I', val)

    def sets(self, addr, val, binary = False):
        if binary:
            addr -= self._base
            if addr+len(val) > len(self._bin):
                self._bin = self._bin + chr(0)*(addr+len(val)-len(self._bin))
            self._bin = self._bin[:addr] + val + self._bin[addr+len(val):]
        else:
            self.set(addr, len(val)+1, '%ds' % (len(val)+1), val+chr(0))

    def splice(self, filename, addr):
        s = file(filename).read()
        addr -= self._base

        if addr+len(s) > len(self._bin):
            self._bin = self._bin + chr(0)*(addr+len(s)-len(self._bin))

        self._bin = self._bin[:addr] + s + self._bin[addr+len(s):]

    def arm_bl(self, addr, where):
        if (addr % 2 > 0) or (where % 2 > 0):
            raise ValueException('Not aligned')

        offset = (where - addr - 8) >> 2
        self.setl(addr, 0xeb000000|offset)

    def thumb_bl(self, addr, where):
        offset = (where - addr - 4) >> 1
        if abs(offset) > pow(2, 22):
            raise ValueExcpetion('Thumb BL: Out of bounds')
        offset = offset & 0x3fffff
        self.setw(addr, 0xf000|(offset >> 11))
        self.setw(addr+2, 0xf800|(offset & 0x7ff))

    def arm_mov_pc(self, addr, dest):
        if (addr % 2 > 0) or (dest % 2 > 0):
            raise ValueException('Not aligned')

        offset = (dest - addr - 8)
        if abs(offset) > pow(2, 11):
            raise ValueExcpetion('ARM MOV PC: Out of bounds')
        offset = offset & 0xfff
        self.setl(addr, 0xe59ff000|offset)
