#!/usr/bin/python

import sys
import time
import select
import tty
import termios
import struct
from bluetooth.bluez import BluetoothSocket
from bluetooth.btcommon import BluetoothError

class MonitorPacket:

    MON_CONNECTED   = 1
    MON_DISCONNECT  = 2
    MON_ERROR       = 3
    MON_READMEM     = 4
    MON_WRITEMEM    = 5
    MON_WRITEFLASH  = 6
    MON_ERASEFLASH  = 7
    MON_REBOOT      = 8
    MON_READREG     = 9
    MON_WRITEREG    = 10
    MON_CALL        = 11
    MON_PLUGIN      = 12

    def __init__(self, cmd = 0, payload = ''):
        (self.cmd, self.payload) = (cmd, payload)

    def __str__(self):
        return '<Packet type=%02x payload=%s>' % (self.cmd, ' '.join(map(hex, map(ord, self.payload))))

    def to_socket(self, sock):
        sock.send(struct.pack('<BB', self.cmd, len(self.payload)) + self.payload)

    def from_socket(self, sock):
        (self.cmd, length) = struct.unpack('<BB', sock.recv(2))
        if length > 0:
            self.payload = sock.recv(length)
        else:
            self.payload = ''

class Monitor(object):

    def connect(self, addr, port = 1):
        (self.addr, self.port) = (addr, port)
        self.s = BluetoothSocket()
        self.s.connect((addr, port))

        # handle "connected" packet
        pkt = MonitorPacket()
        pkt.from_socket(self.s)
        if pkt.cmd != pkt.MON_CONNECTED:
            raise exception('Unexpected packet %s' % pkt)
        self.firmware = pkt.payload

    def reconnect(self):
        self.s = BluetoothSocket()
        self.s.connect((self.addr, self.port))

        # handle "connected" packet
        pkt = MonitorPacket()
        pkt.from_socket(self.s)
        if pkt.cmd != pkt.MON_CONNECTED:
            raise exception('Unexpected packet %s' % pkt)
        self.firmware = pkt.payload

    def read_mem(self, addr, length):
        data = ''
        while len(data) < length:
            to_read = min(length, 64)
            pkt = MonitorPacket()
            pkt.cmd = pkt.MON_READMEM
            pkt.payload = struct.pack('<II', addr + len(data), to_read)
            pkt.to_socket(self.s)

            pkt = MonitorPacket()
            pkt.from_socket(self.s)
            if pkt.cmd != pkt.MON_READMEM:
                print 'Unexpected packet ', pkt
                sys.exit(1)
            data = data + pkt.payload[8:]

        return map(ord, data)

    def write_mem(self, addr, data):
        pkt = MonitorPacket()
        pkt.cmd = pkt.MON_WRITEMEM
        pkt.payload = struct.pack('<II', addr, len(data)) + data
        pkt.to_socket(self.s)

        pkt = MonitorPacket()
        pkt.from_socket(self.s)
        if pkt.cmd != pkt.MON_WRITEMEM:
            print 'Unexpected packet ', pkt
            sys.exit(1)

    def write_flash(self, addr, data):
        pkt = MonitorPacket()
        pkt.cmd = pkt.MON_WRITEFLASH
        pkt.payload = struct.pack('<II', addr, len(data)) + data
        pkt.to_socket(self.s)

        pkt = MonitorPacket()
        pkt.from_socket(self.s)
        if pkt.cmd != pkt.MON_WRITEFLASH:
            print 'Unexpected packet ', pkt
            sys.exit(1)

    def erase_flash(self, addr, length):
        pkt = MonitorPacket()
        pkt.cmd = pkt.MON_ERASEFLASH
        pkt.payload = struct.pack('<II', addr, length)
        pkt.to_socket(self.s)

        pkt = MonitorPacket()
        pkt.from_socket(self.s)
        if pkt.cmd != pkt.MON_ERASEFLASH:
            print 'Unexpected packet ', pkt
            sys.exit(1)

    def reboot(self):
        pkt = MonitorPacket()
        pkt.cmd = pkt.MON_REBOOT
        pkt.to_socket(self.s)

        pkt = MonitorPacket()
        pkt.from_socket(self.s)
        if pkt.cmd != pkt.MON_REBOOT:
            print 'Unexpected packet ', pkt
            sys.exit(1)
        self.s.close()

    def read_reg(self, addr):
        pkt = MonitorPacket()
        pkt.cmd = pkt.MON_READREG
        pkt.payload = struct.pack('<I', addr)
        pkt.to_socket(self.s)

        pkt = MonitorPacket()
        pkt.from_socket(self.s)
        if pkt.cmd != pkt.MON_READREG:
            print 'Unexpected packet ', pkt
            sys.exit(1)
        (addr, data) = struct.unpack('<II', pkt.payload)
        return data

    def write_reg(self, addr, data):
        pkt = MonitorPacket()
        pkt.cmd = pkt.MON_WRITEREG
        pkt.payload = struct.pack('<II', addr, data)
        pkt.to_socket(self.s)

        pkt = MonitorPacket()
        pkt.from_socket(self.s)
        if pkt.cmd != pkt.MON_WRITEREG:
            print 'Unexpected packet ', pkt
            sys.exit(1)

    def call(self, addr, r0 = 0, r1 = 0, r2 = 0, r3 = 0):
        pkt = MonitorPacket()
        pkt.cmd = pkt.MON_CALL
        pkt.payload = struct.pack('<IIIII', addr, r0, r1, r2, r3)
        pkt.to_socket(self.s)

        pkt = MonitorPacket()
        pkt.from_socket(self.s)
        if pkt.cmd != pkt.MON_CALL:
            print 'Unexpected packet ', pkt
            sys.exit(1)
        (addr, r0) = struct.unpack('<II', pkt.payload)
        return r0

    def plugin(self, state):
        pkt = MonitorPacket()
        pkt.cmd = pkt.MON_PLUGIN
        pkt.payload = struct.pack('<I', state)
        pkt.to_socket(self.s)

        pkt = MonitorPacket()
        pkt.from_socket(self.s)
        if pkt.cmd != pkt.MON_PLUGIN:
            print 'Unexpected packet ', pkt
            sys.exit(1)
        (state, ) = struct.unpack('<I', pkt.payload)
        return state

commands = {}

class InteractiveMonitor(Monitor):

    def MonitorCommand(f):
        commands[f.__name__] = f

    @MonitorCommand
    def dm(self, args):
        """Display memory contents. Params: address, length in bytes"""
        (addr, length) = (0, 4)
        try:
            if len(args) > 0:
                addr = int(args[0], 0)
            if len(args) > 1:
                length = int(args[1], 0)
        except:
            print 'Invalid arguments. [addr] [length]'
            return

        data = self.read_mem(addr, length)

        idx = 0
        while len(data) > 0:
            print '  %08x:  ' % (addr+idx) ,
            d = data[0:16]
            print ' '.join(map(lambda x: '%02x' % x, d))
            idx += 16
            data = data[16:]

    @MonitorCommand
    def wm(self, args):
        """Write memory contents. Params: address, data in hex"""
        addr = 0
        data = ''
        try:
            if len(args) > 0:
                addr = int(args[0], 0)
            for s in args[1:]:
                data = data + chr(int(s, 16))
        except:
            print 'Invalid arguments. [addr] [hex string]'
            return

        self.write_mem(addr, data)
        print '  Wrote %d bytes at %08x' % (len(data), addr)

    @MonitorCommand
    def wf(self, args):
        """Write flash memory contents. Params: address, data in hex"""
        addr = 0
        data = ''
        try:
            if len(args) > 0:
                addr = int(args[0], 0)
            for s in args[1:]:
                data = data + chr(int(s, 16))
        except:
            print 'Invalid arguments. [addr] [hex string]'
            return

        self.write_flash(addr, data)
        print '  Wrote %d bytes in flash at %08x' % (len(data), addr)

    @MonitorCommand
    def erase(self, args):
        """Erase flash sector(s). Params: address, length in bytes"""
        addr = 0
        length = 0
        try:
            if len(args) > 0:
                addr = int(args[0], 0)
            if len(args) > 1:
                length = int(args[1], 0)
        except:
            print 'Invalid arguments. [addr] [length]'
            return

        if addr < 0x01000000:
            print 'Address below flash.'
            return

        self.erase_flash(addr, length)
        print '  Erased %d bytes at %08x' % (length, addr)

    @MonitorCommand
    def reboot(self, args):
        """Reboots the module. Monitor will be disconnected."""
        Monitor.reboot(self)
        print '  Rebooting.'
        sys.exit(0)
        #time.sleep(5)

        #print '*** Reconnecting to %s-%d...' % (self.addr, self.port),
        #sys.stdout.flush()

        #try:
        #    self.reconnect()
        #except BluetoothError as e:
        #    print 'failed: %s' % e
        #    sys.exit(1)

        #print 'OK!\n'
        #print 'Firmware:', self.firmware
        #print

    @MonitorCommand
    def rr(self, args):
        """Display hw register. Params: address"""
        addr = None
        try:
            if len(args) > 0:
                addr = int(args[0], 0)
        except:
            print 'Invalid argument. [addr]'
            return

        data = self.read_reg(addr)
        print '  %08x = %08x' % (addr, data)

    @MonitorCommand
    def wr(self, args):
        """Write hw register. Params: address, value in hex"""
        addr = None
        value = None
        try:
            if len(args) > 0:
                addr = int(args[0], 0)
            if len(args) > 1:
                value = int(args[1], 16)
        except:
            print 'Invalid arguments. [addr] [value]'
            return

        self.write_reg(addr, value)
        print '  %08x := %08x' % (addr, value)

    @MonitorCommand
    def call(self, args):
        """Call an arm/thumb function. Params: [addr] [r0] [r1] [r2] [r3]"""
        (addr, r0, r1, r2, r3) = (None, 0, 0, 0, 0)
        try:
            if len(args) > 0:
                addr = int(args[0], 0)
            if len(args) > 1:
                r0 = int(args[1], 0)
            if len(args) > 2:
                r1 = int(args[2], 0)
            if len(args) > 3:
                r2 = int(args[3], 0)
            if len(args) > 4:
                r3 = int(args[4], 0)
        except:
            print 'Invalid arguments'
            return

        r0 = Monitor.call(self, addr, r0, r1, r2, r3)
        print '  Returned %08x' % r0

    @MonitorCommand
    def nvdsinfo(self, args):
        data = ''.join(map(chr, self.read_mem(0x000006a4, 36)))
        (write, erase, initialized, current_page,
                tag_head, blob_head, free_space, page0, page1,
                page_size) = struct.unpack('<IIBBxxIIIIII', data)
        print '  Write proc = %08x' % write
        print '  Erase proc = %08x' % erase
        print '  Initialized = %d' % initialized
        print '  Current page = %d' % current_page
        print '  Tag head = %08x' % tag_head
        print '  Blob head = %08x' % blob_head
        print '  Free space = %d' % free_space
        print '  Page 0 = %08x' % page0
        print '  Page 1 = %08x' % page1
        print '  Page size = %d' % page_size

    @MonitorCommand
    def nvdsread(self, args):
        (tag, size) = (0, 0)
        try:
            if len(args) > 0:
                tag = int(args[0], 0)
            if len(args) > 1:
                size = int(args[1], 0)
        except:
            print 'Invalid arguments'
            return

        r0 = Monitor.call(self, 0x010119f5, tag, size, 0x00011000)
        if r0 == 1:
            print '  File found!'
            data = self.read_mem(0x00011000, size)
            print '  ',
            print ' '.join(map(lambda x: '%02x' % x, data))
        else:
            print '  File not found'

    @MonitorCommand
    def load(self, args):
        if len(args) < 1:
            print 'Need a filename';
            return
        try:
            f = file(args[0])
            img = f.read()
            f.close()
            (magic, flags, text, etext, data, bss, ebss, chksum) = struct.unpack('<IIIIIIII', img[0:32])
            if magic != 0x414d5249:
                raise Exception()
        except:
            print 'Unable to read file'
            return
        imgsize = len(img)
        print '  Image size: %d bytes' % imgsize

        if Monitor.plugin(self, 0) > 0:
            print '  Plugin is running, stopping...'
            Monitor.plugin(self, 2)

        print '  Erasing flash...'
        addr = 0x01060000
        while addr < 0x01060000+imgsize:
            print '  %08x' % addr
            self.erase_flash(addr, 1)
            addr += 0x10000

        print '  Writing to flash...'
        addr = 0x01060000
        while len(img) > 0:
            d = img[0:16]
            print '  %08x' % addr
            self.write_flash(addr, d)
            addr = addr + len(d)
            img = img[16:]

        print '  Starting plugin...',
        if Monitor.plugin(self, 1) > 0:
            print 'okay!'
        else:
            print 'failed.'

    @MonitorCommand
    def dump(self, args):
        (addr, length, filename) = (0, 32, 'dump.bin')
        try:
            if len(args) > 0:
                addr = int(args[0], 0)
            if len(args) > 1:
                length = int(args[1], 0)
            if len(args) > 2:
                filename = args[2]
        except:
            print 'Invalid arguments'
            return
        try:
            f = file(filename, 'w')
        except:
            print 'Unable to open file %s for writing' % filename
            return

        data = self.read_mem(addr, length)
        f.write(''.join(map(chr, data)))
        f.close()

        print 'Dumped %d bytes to %s' % (len(data), filename)

    @MonitorCommand
    def plugin(self, args):
        if len(args) < 1 or args[0] == 'status':
            if Monitor.plugin(self, 0) > 0:
                print 'Plugin is running'
            else:
                print 'Plugin is not running'
        elif args[0] == 'start':
            print 'Starting plugin...',
            if Monitor.plugin(self, 1) > 0:
                print 'okay!'
            else:
                print 'failed'
        elif args[0] == 'stop':
            print 'Stopping plugin'
            Monitor.plugin(self, 2)
        else:
            print 'Unknown action %s' % args[0]

    @MonitorCommand
    def quit(self, args):
        """Disconnect and leave"""
        print 'Bye!'
        self.s.close()
        sys.exit(0)

    @MonitorCommand
    def help(self, args):
        """Print command list"""
        for cmd in sorted(commands.keys()):
            print '  %8s\t%s' % (cmd, commands[cmd].__doc__)

    def invoke(self, l):
        try:
            commands[l[0]](self, l[1:])
        except KeyError:
            print 'Unknown command', l[0]

    def complete(self, text, state):
        for cmd in commands.keys():
            if cmd.startswith(text):
                if not state:
                    return cmd
                else:
                    state -= 1

    def run(self, addr):
        print '*** Connecting to %s-%d...' % (addr, 1),
        sys.stdout.flush()

        try:
            self.connect(addr)
        except BluetoothError as e:
            print 'failed: %s' % e
            sys.exit(1)

        print 'OK!\n'
        print 'Firmware:', self.firmware
        print

        # Try enable readline
        try:
            import readline
            readline.parse_and_bind('tab: complete')
            readline.set_completer(self.complete)
        except:
            pass

        while True:
            l = raw_input('> ').split()
            if len(l) < 1:
                continue
            self.invoke(l)

if __name__ == '__main__':
    if len(sys.argv) > 1:
        addr = sys.argv[1]
    else:
        addr = '00:80:37:14:42:df'
    InteractiveMonitor().run(addr)

