/*
 * Ericsson Bluetooth Baseband Controller
 * Flash interface
 *
 * (c) 2013 <fredrik@z80.se>
 */

#ifndef FLASH_H
#define FLASH_H

unsigned int flash_identify();
int flash_erase(unsigned int addr);
int flash_write(unsigned int addr, unsigned short data);

#endif
