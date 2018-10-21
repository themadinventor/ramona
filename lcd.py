#!/usr/bin/python
import pyio

def lcd_write(p, cs1, cs2, data, rs = True):
    p.GPIOB.PORT = data & 0b00111111
    p.GPIOD.PORT = data & 0b11000000

    c = 0b00000100
    if cs1:
        c |= 0b00000001
    if cs2:
        c |= 0b00000010
    if rs:
        c |= 0b00001000
    p.GPIOC.PORT = c
    p.GPIOC.PORT ^= 0b100
    p.GPIOC.PORT ^= 0b100
    #p.GPIOC.strobe_port(0b100)

p = pyio.PyIO('00:80:37:14:49:4b')

p.GPIOC.PORT = 0b00000111
p.GPIOC.DDR = 0b00001111
p.GPIOB.DDR = 0b00111111
p.GPIOD.DDR = 0b11000000

# display on
lcd_write(p, True, True, 0b00111111, False)

# set y-addr
lcd_write(p, True, True, 0b01000000, False)

# set x-addr
lcd_write(p, True, True, 0b10111000, False)

# write data
for x in xrange(2):
    lcd_write(p, True, True, 0xaa)
    lcd_write(p, True, True, 0x55)
    lcd_write(p, True, True, 0xaa)
    lcd_write(p, True, True, 0x55)

p.close()
