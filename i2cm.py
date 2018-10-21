#!/usr/bin/python

import sys
import select
import tty
import termios
import time
from bluetooth.bluez import BluetoothSocket
from bluetooth.btcommon import BluetoothError

class I2CM(object):


    def __init__(self, bdaddr, port = 2):
        self.s = BluetoothSocket()
        self.s.connect((bdaddr, port))

    def read(self, addr, reg):
        addr = addr | 1
        self.s.send(chr(addr) + chr(reg))
        return ord(self.s.recv(1))

    def write(self, addr, reg, val):
        addr = addr & 0xfe
        self.s.send(chr(addr)+chr(reg)+chr(val))

    def close(self):
        self.s.close()

if __name__ == '__main__':
    addr = '00:80:37:14:49:47'

    print '*** Connecting to %s...' % addr,
    sys.stdout.flush()

    try:
        p = I2CM(addr)
    except BluetoothError as e:
        print 'failed: %s' % e
        sys.exit(1)

    print 'OK!\n'

    p.write(0x08, 0xa5, 0x33)

    print 'read %02x' % p.read(0x08, 0xcc)

    p.close()
