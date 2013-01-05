#ifndef __FLASH_H
#define __FLASH_H

void Flash_ErasePage(void *addr);
void Flash_Write(void *dst, void *src, size_t len);

#endif
