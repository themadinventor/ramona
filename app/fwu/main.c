#include "flash.h"
#include "uart.h"
#include "utils.h"

#define FLASH_ID            0x00898892 /* Intel TE28F800B3T, 8 Mbit */
#define FLASH_BASE          0x01000000
#define FLASH_FIRST_BASE    0x01010000
#define FLASH_LIMIT         0x01060000
#define FLASH_PAGE_SIZE     0x00010000

#define OS_MAGIC            0xe1a01007
#define OS_MAX_SIZE         0x00060000

extern unsigned short ROM_CRC16(void *ptr, unsigned int len, unsigned short crc);

extern void *_text, *_etext, *_data, *_edata;

struct fwheader {
    unsigned int magic;
    unsigned int checksum;
    unsigned int reserved[3];
    unsigned int size;
};

void start(void)
{
    *((unsigned char *) 0x00800110) |= 0x02;

    UARTInit();
    UART2SetBaudRate(UART_115200);
    UART2WriteString("IRMA FWU\n\r");

    void *payload_base = &_etext + (&_edata - &_data);

    struct fwheader *header = payload_base;

    if (header->magic != OS_MAGIC) {
        UART2WriteString("Fatal: Wrong magic\n\r");
        goto boot;
    }
   
    if (header->size > OS_MAX_SIZE) {
        UART2WriteString("Fatal: Firmware is too large\n\r");
        goto boot;
    }

    //UART2WriteString("Calculating checksum...\n\r");
    unsigned short checksum = ROM_CRC16(payload_base+8, header->size-8, 0);
    if (checksum != header->checksum) {
        UART2WriteString("Fatal: Invalid checksum\n\r");
        goto boot;
    }

    //UART2WriteString("Flash stuff...\n\r");

    /* Apply voodoo to unlock flash */
    *((unsigned char *) 0x00800308) = 0x1e;
    *((unsigned char *) 0x0080030c) = 0x1e;
    *((unsigned char *) 0x00800310) = 0x16;

    unsigned int flash = flash_identify();
    if (flash != FLASH_ID) {
        UART2WriteString("Fatal: Unknown flash (");
        WriteHex(flash);
        UART2WriteString(")\n\r");
        goto boot;
    }

    //UART2WriteString("Erasing flash...\n\r");
    //for (unsigned int addr = FLASH_FIRST_BASE; addr < FLASH_LIMIT; addr += FLASH_PAGE_SIZE) {
    for (unsigned int addr = FLASH_BASE; addr < FLASH_LIMIT; addr += FLASH_PAGE_SIZE) {
        //WriteHex(addr);
        
        if (flash_erase(addr)) {
            UART2WriteString("Fatal: Failed to erase page\n\r");
            goto reboot;
        //} else {
            //UART2WriteString(" ok\n\r");
        }

        *((unsigned char *) 0x00800110) ^= 0x02;
    }
    //UART2WriteString("Flash empty\n\r");
    //goto leave;

    //UART2WriteString("Writing to flash...");
    //for (unsigned int addr = FLASH_FIRST_BASE; addr < FLASH_FIRST_BASE + imgsize; addr += 2) {
    unsigned short *src = payload_base;
    for (unsigned int addr = FLASH_BASE; addr < FLASH_BASE + header->size; addr += 2) {
        flash_write(addr, *src++); 

        if (!(addr & 0x00000fff)) {
            //WriteHex(addr);
            //UART2WriteString("\n\r");
            *((unsigned char *) 0x00800110) ^= 0x02;
        }
    }

    // Destroy our signature so we won't run again
    flash_write(FLASH_LIMIT, 0); 

reboot:
    //UART2WriteString("\n\rWill reboot.\n\r");

    *((unsigned char *) 0x00800110) |= 0x02;

    *((unsigned int *) 0x00800c0c) = 0xc0;
    *((unsigned int *) 0x00800c0c) = 0x1f;

    *((unsigned char *) 0x00800910) = 0x00;
    *((unsigned char *) 0x00800914) = 0x04;
    *((unsigned char *) 0x00800910) = 0x01;

    for (;;) ;

boot:
    /* Flush UART */
    //while (UART2GetTxFIFOSize() < 127) ;
    //for (int i=0; i<0xfffff; i++) ;
    return;
}

