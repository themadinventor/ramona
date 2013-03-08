/*
 * Ericsson Baseband Controller
 * IRMA Firmware Upgrade
 *
 * 2013-01-08 <fredrik@etf.nu>
 */

#include "uart.h"

extern unsigned short ROM_CRC16(void *ptr, unsigned int len, unsigned short crc);

#define PLUGIN_BASE 0x01060000
#define PLUGIN_LIMIT 0x90000

#define PLUGIN_MAGIC 0x44465755
#define PLUGIN_CRC_1 28
#define PLUGIN_CRC_2 32

struct plugin {
    unsigned int magic;
    unsigned int flags;
    unsigned int text;
    unsigned int etext;
    unsigned int data;
    unsigned int bss;
    unsigned int ebss;
    unsigned int checksum;
    char entry[0];
};

void TryFirmwareUpgrade(void)
{
    struct plugin *plugin = (struct plugin *) PLUGIN_BASE;

    // Disable watchdog?
    *((unsigned char *) 0x00800c0c) = 0xc0;
    *((unsigned char *) 0x00800c0c) = 0x00;

    /*UARTInit();
    UART2SetBaudRate(UART_115200);
    UART2WriteString("TryFirmwareUpgrade\n\r");*/

    if (plugin->magic != PLUGIN_MAGIC) {
        //UART2WriteString("Invalid magic\n\r");
        return;
    }

    unsigned int plugin_size = plugin->etext - plugin->text + plugin->bss - plugin->data;
    if (plugin_size > PLUGIN_LIMIT) {
        //UART2WriteString("Plugin too large\n\r");
        //for (;;) ;
        return;
    }

    //UART2WriteString("Calculating checksum...\n\r");
    unsigned short crc = ROM_CRC16(plugin, PLUGIN_CRC_1, 0);
    crc = ROM_CRC16(plugin->entry, plugin_size-PLUGIN_CRC_2, crc);

    if (crc != plugin->checksum) {
        //UART2WriteString("Invalid checksum\n\r");
        //for (;;) ;
        return;
    }

    /*UART2WriteString("Initializing memory...\n\r");
    WriteHex(plugin->etext); UART2WriteString("\n\r");
    WriteHex(plugin->data); UART2WriteString("\n\r");
    WriteHex(plugin->bss); UART2WriteString("\n\r");
    WriteHex(plugin->ebss); UART2WriteString("\n\r");
    return;*/

    register char *src = (char *) plugin->etext;
    register char *dst = (char *) plugin->data;
    while (dst < (char *) plugin->bss)
        *dst++ = *src++;
    while (dst < (char *) plugin->ebss)
        *dst++ = 0;

    //UART2WriteString("FWU good, would yield\n\r");

    int (*proc)(int) = (void *) plugin->entry + 1;
    proc(0);
}

