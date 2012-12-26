#!/usr/bin/python
#
# Ericsson Bluetooth Baseband Controller
# Firmware Patching Tool
#
# (c) 2012 <fredrik@z80.se>

import patcher
import sys
import time
import os

p = patcher.Patcher('../flash_rok101008_mod.bin', 0x01000000)

print 'Hash: %s' % p.hash
print 'Size: %d' % p.size
print 'Generated: %s' % p.gets(0x01010030)[:16]
print 'Comment: %s' % p.gets(0x01010050)

print

if p.hash != '0f67f559567781fe6109a75544f0a74a':
    print 'Incorrect hash!'
    sys.exit(1)

# Set default baudrates
p.setb(0x0103dfbc, 0x02)    # UART1 Baudrate = 0x14 -> 9600
p.setb(0x0103dc80, 0x02)    # UART2 Baudrate = 0x14 -> 9600

# Set revision information
p.sets(0x01010030, time.strftime('%Y-%m-%d %H:%M'))
p.sets(0x01010050, '%s@%s' % (os.environ['USER'], os.uname()[1]))

# Inject backpack
p.splice('../backpack/backpack.bin', 0x01000000);
#p.thumb_bl(0x01043232, 0x01000010); # init
p.thumb_bl(0x01040334, 0x01000010); # divide by zero
p.thumb_bl(0x0103dec6, 0x01000012); # uart2 rx int

# Inject process descriptor
pd = p.getl(0x01000014)
print 'Descriptor at %08x' % pd
p.setl(0x01043da4, pd)

# Register process PCB
pcb = p.getl(0x01000018)
print 'PCB at %08x' % pcb
p.setl(0x01043d48, pcb)

# Save firmware hash in header
fwh = p.blobhash[:4]
p.sets(0x01000004, fwh)

print 
print 'Hash: %s' % p.hash
print 'Size: %d' % p.size
print 'Generated: %s' % p.gets(0x01010030)
print 'Comment: %s' % p.gets(0x01010050)
print

p.save('../flash_rok101008_nodfu.bin')
