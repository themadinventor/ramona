/*
 * Ericsson Bluetooth Baseband Controller
 * OTA FWU Main
 *
 * (c) 2013 <fredrik@z80.se>
 */

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
#ifdef DEBUG_LED
    // Turn off LED
    *((unsigned char *) 0x00800110) |= 0x02;
#endif

#ifdef DEBUG_UART
    // Initialize UART and write stuff (not really working, is buffering)
    UARTInit();
    UART2SetBaudRate(UART_115200);
    UART2WriteString("IRMA FWU\n\r");
#endif

    // Calculate a pointer to the payload
    void *payload_base = &_etext + (&_edata - &_data);
    struct fwheader *header = payload_base;

    // Sanuty checks
    if (header->magic != OS_MAGIC) {
#ifdef DEBUG_UART
        UART2WriteString("Fatal: Wrong magic\n\r");
#endif
        goto boot;
    }
   
    if (header->size > OS_MAX_SIZE) {
#ifdef DEBUG_UART
        UART2WriteString("Fatal: Firmware is too large\n\r");
#endif
        goto boot;
    }

    // Verify checksum
    unsigned short checksum = ROM_CRC16(payload_base+8, header->size-8, 0);
    if (checksum != header->checksum) {
#ifdef DEBUG_UART
        UART2WriteString("Fatal: Invalid checksum\n\r");
#endif
        goto boot;
    }

    /* Apply voodoo to unlock flash */
    *((unsigned char *) 0x00800308) = 0x1e;
    *((unsigned char *) 0x0080030c) = 0x1e;
    *((unsigned char *) 0x00800310) = 0x16;

    // Check that we support this flash
    unsigned int flash = flash_identify();
    if (flash != FLASH_ID) {
#ifdef DEBUG_UART
        UART2WriteString("Fatal: Unknown flash (");
        WriteHex(flash);
        UART2WriteString(")\n\r");
#endif
        goto boot;
    }

    // Erase OS
    for (unsigned int addr = FLASH_BASE; addr < FLASH_LIMIT; addr += FLASH_PAGE_SIZE) {
        if (flash_erase(addr)) {
#ifdef DEBUG_UART
            UART2WriteString("Fatal: Failed to erase page\n\r");
#endif
            goto reboot;
        }

#ifdef DEBUG_LED
        // Toggle LED to indicate activity
        *((unsigned char *) 0x00800110) ^= 0x02;
#endif
    }

    // Write new OS
    unsigned short *src = payload_base;
    for (unsigned int addr = FLASH_BASE; addr < FLASH_BASE + header->size; addr += 2) {
        flash_write(addr, *src++); 

#ifdef DEBUG_LED
        // Toggle LED each 4 kB to indicate activity
        if (!(addr & 0x00000fff)) {
            *((unsigned char *) 0x00800110) ^= 0x02;
        }
#endif
    }

    // Destroy our signature so we won't run again
    flash_write(FLASH_LIMIT, 0); 

reboot:
#ifdef DEBUG_LED
    // Turn off LED
    *((unsigned char *) 0x00800110) |= 0x02;
#endif

    // Trigger watchdog
    *((unsigned int *) 0x00800c0c) = 0xc0;
    *((unsigned int *) 0x00800c0c) = 0x1f;

    // Required for watchdog to work. Wtf.
    *((unsigned char *) 0x00800910) = 0x00;
    *((unsigned char *) 0x00800914) = 0x04;
    *((unsigned char *) 0x00800910) = 0x01;

    // Await death.
    for (;;) ;

boot:
    // Optionally flush the UART here if we get it to work
    return;
}

