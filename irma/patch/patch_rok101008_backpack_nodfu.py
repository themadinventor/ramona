#!/usr/bin/python
#
# Ericsson Bluetooth Baseband Controller
# Firmware Patching Tool
#
# (c) 2012 <fredrik@z80.se>

import patcher
import symlist
import sys
import time
import os

p = patcher.Patcher('../flash_rok101008_mod.bin', 0x01000000)
s = symlist.Symlist('../backpack/backpack.sym')

print 'Hash: %s' % p.hash
print 'Size: %d' % p.size
print 'Generated: %s' % p.gets(0x01010030)[:16]
print 'Comment: %s' % p.gets(0x01010050)

print

if p.hash != '0f67f559567781fe6109a75544f0a74a':
    print 'Incorrect hash!'
    sys.exit(1)

# Set default baudrates
p.setb(0x0103dfbc, 0x02)    # UART1 Baudrate = 0x02 -> 115200 (0x14 -> 9600)
p.setb(0x0103dc80, 0x02)    # UART2 Baudrate = 0x02 -> 115200 (0x14 -> 9600)

# Set revision information
p.sets(0x01010030, time.strftime('%Y-%m-%d %H:%M'))
p.sets(0x01010050, '%s@%s' % (os.environ['USER'], os.uname()[1]))

# Inject backpack
p.splice('../backpack/backpack.bin', 0x01000000)
p.thumb_bl(0x01040334, s.syms['div_by_zero']) # divide by zero
p.thumb_bl(0x0103dec6, s.syms['uart2_rx_int']) # uart2 rx int

p.setl(0x01043f8c, s.syms['Flash_Eraser']+1) # flash eraser

p.thumb_bl(0x010118f0, s.syms['Flash_Handler']) # flash handler
p.setw(0x010118ee, 0x9802)

p.setl(0x01042064, s.syms['panic']) # ose crash handler

# Inject process descriptor
p.setl(0x01043da4, s.syms['backpackProc'])

# Register process PCB
p.setl(0x01043d48, s.syms['procmem']+2048)

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
