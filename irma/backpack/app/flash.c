/*
 * Ericsson Bluetooth Baseband Controller
 * Improved NVDS Implementation
 *
 * (c) 2012 <fredrik@z80.se>
 */

#include "host/ose.h"
#include "host/app.h"
#include "app/signals.h"

#include <stdio.h>

/*void Flash_Handler(void)
{
    const SIGSELECT anysig[] = {0};

    for (;;) {
        SIGNAL *s = OSE_receive((SIGSELECT *) anysig);

        switch (s->sig_no) {
        case SIG_NVDS_WRITE:
            break;

        case SIG_NVDS_READ:
            break;

        case SIG_NVDS_DELETE:
            break;

        default:
            OSE_UserError(0x20);
        }
    }
}*/

void Flash_Handler(SIGNAL *s)
{
    if (s->sig_no == SIG_FLASH_WRITE) {
        struct sig_write *p = (void *) s;

        // Unlock flash (wtf?!)
        *((unsigned int *)0x00800314) &= 0xef;
        while (p->len) {
            NVDS_Context.write(p->dst, *((char *)p->src));
            p->dst++, p->src++, p->len--;
        }
        // Lock flash (yeah!)
        *((unsigned int *)0x00800314) |= 0x10;

        SIGNAL *r = OSE_alloc(4, SIG_FLASH_COMPLETE);
        OSE_send(&r, OSE_sender(&s));
    } else {
        OSE_UserError(0x20);
    }
}

void Flash_Eraser(void)
{
    const SIGSELECT anysig[] = {0};

    for (;;) {
        SIGNAL *s = OSE_receive((SIGSELECT *) anysig);

        switch (s->sig_no) {
        case SIG_NVDS_ERASE:
            {
                unsigned char page = s->raw[2];
                NVDS_Context.erase(NVDS_Context.pages[page]);
                page = !page;
                NVDS_Context.write(NVDS_Context.pages[page]+2, 0x03);
            }
            break;

        case SIG_FLASH_ERASE:
            {
                struct sig_erase *p = (void *) s;
                
                // Unlock flash (wtf?!)
                *((unsigned int *)0x00800314) &= 0xef;
                NVDS_Context.erase(p->addr);
                // Lock flash (yeah!)
                *((unsigned int *)0x00800314) |= 0x10;

                SIGNAL *r = OSE_alloc(4, SIG_FLASH_COMPLETE);
                OSE_send(&r, OSE_sender(&s));
            }
            break;

        default:
            OSE_UserError(0x20);
        }

        OSE_free_buf(&s);
    }
}

void Flash_ErasePage(void *addr)
{
    SIGNAL *s = OSE_alloc(sizeof(struct sig_erase), SIG_FLASH_ERASE);
    struct sig_erase *p = (void *) s;
    p->addr = addr;
    OSE_send(&s, proc_flash_eraser);

    const SIGSELECT response[] = {1, SIG_FLASH_COMPLETE};
    s = OSE_receive((SIGSELECT *) response);
    OSE_free_buf(&s);
}

void Flash_Write(void *dst, void *src, size_t len)
{
    SIGNAL *s = OSE_alloc(sizeof(struct sig_write), SIG_FLASH_WRITE);
    struct sig_write *p = (void *) s;
    p->dst = dst;
    p->src = src;
    p->len = len;
    OSE_send(&s, proc_flash_handler);

    const SIGSELECT response[] = {1, SIG_FLASH_COMPLETE};
    s = OSE_receive((SIGSELECT *) response);
    OSE_free_buf(&s);
}
