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

void Flash_Handler(SIGNAL *s)
{
    if (s->sig_no == SIG_FLASH_WRITE) {
        struct sig_flash_write *p = (void *) s;

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

void Flash_Response(SIGNAL *s)
{
    SIGNAL *r = OSE_alloc(4, SIG_FLASH_COMPLETE);
    OSE_send(&r, OSE_sender(&s));
}

void Flash_Response_Wrap(SIGNAL *s);
asm (
    "Flash_Response_Wrap:\n"    \
    "mov r2, sp\n"              \
    "ldr r3, =0xef8\n"          \
    "mov sp, r3\n"              \
    "push {r2, lr}\n"           \
    "bl Flash_Response\n"       \
    "pop {r2, r3}\n"            \
    "mov sp, r2\n"              \
    "bx r3"
    );

static SIGSELECT anysig[] = {0};

__attribute__((noreturn)) void Flash_Eraser(void)
{

    for (;;) {
        //UART2WriteString("eraser: listening\n\r");
        SIGNAL *s = OSE_receive((SIGSELECT *) anysig);
        //UART2WriteString("eraser: got signal\n\r");

        switch (s->sig_no) {
        case SIG_NVDS_ERASE:
            {
                register unsigned char page = s->raw[2];
                NVDS_Context.erase(NVDS_Context.pages[page]);
                page = !page;
                NVDS_Context.write(NVDS_Context.pages[page]+2, 0x03);
            }
            break;

        case SIG_FLASH_ERASE:
            {
                //UART2WriteString("eraser: doin' it\n\r");

                // Unlock flash (wtf?!)
                *((unsigned int *)0x00800314) &= 0xef;
                NVDS_Context.erase(((struct sig_flash_erase *) s)->addr);
                // Lock flash (yeah!)
                *((unsigned int *)0x00800314) |= 0x10;

                //UART2WriteString("eraser: done\n\r");

                Flash_Response_Wrap(s);

                //UART2WriteString("eraser: response sent\n\r");
            }
            break;

        default:
            OSE_UserError(0x20);
        }

        OSE_free_buf(&s);
        //UART2WriteString("eraser: freeing\n\r");
        //UART2WriteString("eraser: done\n\r");
    }
}

void Flash_ErasePage(void *addr)
{
    //printf("%s: enter\n", __FUNCTION__);

    SIGNAL *s = OSE_alloc(sizeof(struct sig_flash_erase), SIG_FLASH_ERASE);
    struct sig_flash_erase *p = (void *) s;
    p->addr = addr;
    OSE_send(&s, proc_flash_eraser);

    //printf("%s: signal sent\n", __FUNCTION__);

    const SIGSELECT response[] = {1, SIG_FLASH_COMPLETE};
    s = OSE_receive((SIGSELECT *) response);
    OSE_free_buf(&s);

    //printf("%s: got answer\n", __FUNCTION__);
}

void Flash_Write(void *dst, void *src, size_t len)
{
    SIGNAL *s = OSE_alloc(sizeof(struct sig_flash_write), SIG_FLASH_WRITE);
    struct sig_flash_write *p = (void *) s;
    p->dst = dst;
    p->src = src;
    p->len = len;
    OSE_send(&s, proc_flash_handler);

    const SIGSELECT response[] = {1, SIG_FLASH_COMPLETE};
    s = OSE_receive((SIGSELECT *) response);
    OSE_free_buf(&s);
}
