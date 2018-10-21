#!/usr/bin/python

import sys
import time
from random import random
from bluetooth.bluez import BluetoothSocket
from bluetooth.btcommon import BluetoothError

addr = '00:80:37:14:49:42'
port = 2

print '*** Connecting to %s-%d...' % (addr, port),
sys.stdout.flush()

try:
    s = BluetoothSocket()
    s.connect((addr, port))
except BluetoothError as e:
    print 'failed: %s' % e
    sys.exit(1)

print 'OK!\n'

idx = 0
while True:
    frame = []
    for i in xrange(15):
        frame = frame + [int(random()*256)]
    frame = ''.join(map(chr, map(lambda x : x |0x80, frame)))
    s.send('b'+frame+'\n')
    time.sleep(.05)
    idx += 1
