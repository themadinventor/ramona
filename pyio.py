#!/usr/bin/python

import sys
import select
import tty
import termios
import time
from bluetooth.bluez import BluetoothSocket
from bluetooth.btcommon import BluetoothError

GPIO_SET_PORT = 0
GPIO_SET_DDR = 1
GPIO_STROBE_PORT = 2
GPIO_STROBE_DDR = 3
GPIO_READ_PIN = 4

EEPROM_ADDR = 5
EEPROM_READ = 6
EEPROM_WRITE = 7

SPI_CONFIG = 8
SPI_XCHG = 9

ADC_CONFIG = 10
ADC_READ = 11
ADC_READL = 12

OW_RESET = 13
OW_READ = 14
OW_WRITE = 15

REG_ADDR = 16
REG_READ = 17
REG_WRITE = 18

FW_VERSION = 19
FW_BOOTLOADER = 20

GPIO_PORTB = 0
GPIO_PORTC = 1
GPIO_PORTD = 2

OW_BIT = 0
OW_BYTE = 1

class PyIOGPIO(object):

    def __init__(self, parent, gpio):
        self.p = parent
        self.gpio = gpio
        self.port = 0x00
        self.ddr = 0x00

    @property
    def PORT(self):
        return self.port

    @PORT.setter
    def PORT(self, val):
        self.port = val
        self.p.__cmd__(GPIO_SET_PORT, self.gpio, self.port)

    def strobe_port(self, bits):
        self.p.__cmd__(GPIO_STROBE_PORT, self.gpio, bits)

    @property
    def DDR(self):
        return self.ddr

    @DDR.setter
    def DDR(self, val):
        self.ddr = val
        self.p.__cmd__(GPIO_SET_DDR, self.gpio, self.ddr)

    def strobe_ddr(self, bits):
        self.p.__cmd__(GPIO_STROBE_DDR, self.gpio, bits)

    @property
    def PIN(self):
        return self.p.__cmd__(GPIO_READ_PIN, self.gpio, read = True)

class PyIO(object):


    def __init__(self, bdaddr, port = 2):
        self.s = BluetoothSocket()
        self.s.connect((bdaddr, port))

        self.GPIOB = PyIOGPIO(self, GPIO_PORTB)
        self.GPIOC = PyIOGPIO(self, GPIO_PORTC)
        self.GPIOD = PyIOGPIO(self, GPIO_PORTD)

    def __cmd__(self, op, arg = 0x00, data = 0x00, read = False):
        if read:
            self.s.send(chr((op << 3)|arg))
            return ord(self.s.recv(1))
        else:
            self.s.send(chr(0x80|(op << 3)|arg)+chr(data))

if __name__ == '__main__':
    addr = '00:80:37:14:49:4b'

    print '*** Connecting to %s...' % addr,
    sys.stdout.flush()

    try:
        p = PyIO(addr)
    except BluetoothError as e:
        print 'failed: %s' % e
        sys.exit(1)

    print 'OK!\n'

    p.GPIOC.DDR |= 0x0c
    p.GPIOC.PORT |= 0x01

    while True:
        if p.GPIOC.PIN & 0x01:
            p.GPIOC.PORT = 0x08|1
        else:
            p.GPIOC.PORT = 0x04|1
        #time.sleep(.1)

    s.close()
