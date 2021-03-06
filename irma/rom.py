#!/usr/bin/python
#
# Ericsson Bluetooth Baseband Controller
# ROM Bootloader toolkit
#
# (c) 2012 <fredrik@z80.se>

import sys
import serial
import struct
import hashlib
import time

class IRMAFlasher:

    def __init__(self):
        self.s = serial.Serial('/dev/ttyUSB1', 9600, timeout = 0.01)
        self.rs = serial.Serial('/dev/ttyUSB0')

    def blobhash(self, filename):
        b = file(filename).read()
        m = hashlib.md5()
        m.update(b[8:])
        return m.hexdigest()[:8]

    def asicNameById(self, id):
        if id == 0x01:
            return 'Irma-B P1A'
        elif id == 0x02:
            return 'Irma-B P2A'
        elif id == 0x03:
            return 'Irma-B P3A'
        elif id == 0x04:
            return 'Irma-B P3B'
        elif id == 0x05:
            return 'Irma-B P4A'
        elif id == 0x06:
            return 'Irma-B P5A'
        elif id == 0x07:
            return 'Irma-C P1A'
        elif id == 0x08:
            return 'Irma-B P4B'
        elif id == 0x09:
            return 'Irma-B P4C'
        elif id == 0x0B:
            return 'Irma-C P1B'
        elif id == 0x0D:
            return 'Irma-C P2A'
        elif id == 0x10:
            return 'Blink PBM 990 90/2'
        else:
            return 'Unknown (%08x)' % id

    # Perform a hard reset on the module by pulling /RESET
    def hard_reset(self):
        self.rs.setRTS(True)
        self.rs.setRTS(False)
        self.rs.setRTS(True)

    # Try invoke the ROM bootloader by repeatedly pulling /RESET
    # and sending the magic word until we get a response.
    def connect(self):
        sys.stdout.write('Detecting hardware... ')

        spinner = '/-\|'
        spinidx = 0

        while True:
            self.hard_reset()
            
            sys.stdout.write('\b' + spinner[spinidx])
            sys.stdout.flush()
            spinidx = (spinidx + 1) % 4

            for _ in xrange(40):
                self.s.write("A\x55\x33")
                d = self.s.read(9)
                if len(d) == 9 and d[8] == 'R':
                    (self.romrev, self.flashrev, c) = struct.unpack('<IIc', d)
        
                    print '\bfound!\n'
                    print 'Chip Revision:  %s\nFlash Revision: %08x\n' % (self.asicNameById(self.romrev), self.flashrev)

                    self.s.timeout = None
                    return

    # Send a generic blob while drawing a progress bar
    def write_blob(self, data):
        size = len(data)
        written = 0
        while True:
            self.s.write(data[:256])
            written += len(data[:256])
            data = data[256:]

            width = 32
            sys.stdout.write('\r[');
            dots = width*written/size
            for i in xrange(width):
                if i < dots:
                    sys.stdout.write('=')
                else:
                    sys.stdout.write(' ')
            sys.stdout.write('] %3d %%' % (100*written/size))

            sys.stdout.flush()
            if len(data) == 0:
                break

        sys.stdout.write('\n')

    # Download a blob, preceded by an offset-size tuple
    def load(self, filename, addr = 0):
        print 'Loading %s...' % filename
        f = file(filename)
        boot = f.read()
        bootsz = len(boot)
        print 'Downloading (%d bytes)...' % bootsz
        self.s.write(struct.pack('<II', addr, bootsz))
        self.write_blob(boot[:bootsz])

    # Download the RAM bootloader
    def bootstrap(self, filename='irma_fl_ebt_1_00_(ecs412mod).axf'):
        print 'Bootstrapping %s...' % filename
        f = file(filename)

        # Parse AIF header
        boot = f.read()
        (entry, rosz, rwsz) = struct.unpack('<12xI4xII', boot[:28])
        entry = (entry & 0xffffff) << 2
        print '  entry=0x%08x, ro=%04x, rw=0x%04x\n' % (entry, rosz, rwsz)
        bootsz = rosz+rwsz

        print 'Downloading bootstrap (%d bytes)...' % bootsz

        self.s.write(struct.pack('<II', 0x00000000, bootsz))
        self.write_blob(boot[:bootsz])

        d = self.s.read(1)
        if d != '>':
            raise Exception('Unexpected bootstrap response "%s"' % d)

        print 'Bootstrap is running'

    def command(self, cmd, params = ""):
        self.s.write(cmd+params)

        #while True:
        d = self.s.readline()
        self.s.read(2) #vask
        return d[:-2]

    def version(self):
        return self.command('V')

    def memory(self):
        return self.command('M')

    def erase(self, bank = 0):
        if bank == 0:
            print 'Erasing program banks'
        elif bank == 1:
            print 'Erasing parameter banks'
        elif bank == 2:
            print 'Erasing all banks'
        else:
            raise ValueException('Invalid bank')

        return self.command('D%d' % bank)

    def read(self, addr):
        return self.command('L%06x' % int(addr))

    def set_baud(self, baud):
        print 'Setting baud rate to %d' % baud

        if baud == 4800:
            cmd = 'C0'
        elif baud == 9600:
            cmd = 'C1'
        elif baud == 19200:
            cmd = 'C2'
        elif baud == 38400:
            cmd = 'C3'
        elif baud == 57600:
            cmd = 'C4'
        elif baud == 115200:
            cmd = 'C5'
        elif baud == 230400:
            cmd = 'C6'
        elif baud == 460800:
            cmd = 'C7'
        else:
            raise ValueException('Invalid baud rate %d' % baud)

        self.s.write(cmd)
        self.s.read(1)
        self.s.baudrate = baud
        self.s.write('!')
        d = self.s.readline()
        self.s.read(2) #vask
        return d[:-2]

    def program(self, filename):
        print 'Programming %s to flash...' % filename

        f.s.write('P')
        f.load(filename, 0x01000000)

        d = f.s.readline()
        f.s.read(2) #vask
        if d[:-2] != '!:':
            raise Exception('Flashing failed! Returned "%s"' % d[:-2])
        else:
            print 'Done.'
    
        f.status()

    def status(self):
        s = f.command('E')

        if s == 'E9':
            print 'No errors.'
        elif s == 'E0':
            raise Exception('Invalid command')
        elif s == 'E2':
            raise Exception('Program error')
        elif s == 'E3':
            raise Exception('Erase error')
        elif s == 'E4':
            raise Exception('Flash image too large')
        elif s == 'E5':
            raise Exception('Flash not erased')
        elif s == 'E6':
            raise Exception('Unknown device')
        else:
            raise Exception('Unknown status %s' % s)

    def leave(self):
        return f.s.write('Q')

    def terminal(self, baudrate=9600, hex=False):
        self.s.baudrate = baudrate
        while True:
            d = self.s.read(1)
            if len(d) < 1:
                continue
            if hex:
                sys.stdout.write('%02x ' % ord(d[0]))
            else:
                sys.stdout.write(d)
            sys.stdout.flush()

    def flash(self, filename):
        self.connect()

        fwrev = self.blobhash(filename)
        print 'Firmware to download: %s' % fwrev

        if fwrev == '%08x' % self.flashrev:
            print 'Same version! (%08x)' % self.flashrev

        self.bootstrap()

        print
        print 'Version: %s' % f.version()
        print 'Flash: %s' % f.memory()
        print

        self.set_baud(460800)
        self.erase()
        self.program(filename)

        print 'Rebooting...'
        self.leave()

if __name__ == '__main__':
    f = IRMAFlasher()

    f.flash('flash_rok101008_nodfu.bin')

    f.terminal()
