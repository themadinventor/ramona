#ifndef FLASH_H
#define FLASH_H

unsigned int flash_identify();
int flash_erase(unsigned int addr);
int flash_write(unsigned int addr, unsigned short data);

#endif
