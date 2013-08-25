/*
 * Ericsson Bluetooth Baseband Controller
 * Backpack Main
 *
 * (c) 2012 <fredrik@z80.se>
 */

#include "host/ose.h"
#include "host/app.h"
#include "host/irma.h"

#include "uart.h"
#include "utils.h"
#include "signals.h"
#include "transport.h"
#include "plugin.h"

#include <stdio.h>

#include "lwbt/phybusif.h"
#include "lwbt/lwbt_memp.h"
#include "lwbt/hci.h"
#include "lwbt/l2cap.h"
#include "lwbt/sdp.h"
#include "lwbt/rfcomm.h"
#include "lwip/memp.h"
#include "lwip/mem.h"
#include "lwip/sys.h"
#include "lwip/stats.h"

#include "btstack.h"

void bpMain(void);

// use a struct to make sure they stay in order.
// failing to do so will result in crashes or undefined behaviour,
// as OSE will initialize the memory between those areas and
// thus overwrite other application variables.
struct {
    char stack[1024]; // those sizes are hardcoded in head.S
    char stack2[1024];
    char bpPCB[92];
} procmem;

/*
 * This structure defines our process so
 * OSE can schedule it and pass signals to it.
 * Additional magic is done in head.S and the patching script.
 */
const PROCINIT backpackProc = {
    .type = 0,
    .entry = bpMain,
    .prio = 4,
    .readyq = (void *)0x73d0, // ready queue for this priority. wrong (=other) value will crash OSE.
    .pcb = procmem.bpPCB,
    .stack_limit = procmem.stack+sizeof(procmem.stack),
    .stack_base = procmem.stack,
    .pid = PID_BACKPACK,
    .name = "backpack"
};

/*
 * This gets called through the OSE interrupt handling mechanism.
 */
void uart2_rx_int(int bytes)
{
    SIGNAL *s = OSE_alloc(3+bytes, SIG_UART_RX);

    s->raw[2] = bytes;
    int idx = 0;
    while (bytes--) {
        s->raw[3+idx++] = UART2ReadByte();
    }

    OSE_send(&s, PID_BACKPACK);
}

void (*uart2_rx_handler)(int bytes, char *data);

void set_uart2_rx_handler(void *proc)
{
    uart2_rx_handler = proc;
}

void (*tick_handler)(void);

void set_tick_handler(void *proc)
{
    tick_handler = proc;
}

int (*signal_handler)(SIGNAL *s);

void set_signal_handler(void *proc)
{
    signal_handler = proc;
}

void btstack_init_complete(void)
{
    char autostart;
    if (plugin_is_autostart() || (NVDS_ReadFile(0x80, 1, &autostart) && autostart)) {
        plugin_enable();
    }
}

/*
 * This is the entry point of our thread
 */
void bpMain(void)
{
    lwbt_init();
    monitor_init();

    //I2C_Init();

    timer_add(1000, SIG_TIMER_1S);

    static const SIGSELECT anysig[] = {0};
    for(;;) {
        SIGNAL *s = OSE_receive((SIGSELECT *) anysig);

        if (signal_handler) {
            if (signal_handler(s)) {
                OSE_free_buf(&s);
                continue;
            }
        }

        switch (s->sig_no) {
        case SIG_TIMER_1S:
            lwbt_timer();
            timer_add(1000, SIG_TIMER_1S);

            if (tick_handler) {
                tick_handler();
            }
#if 0
            UART2_MCR &= ~0x02;
            timer_add(100, SIG_TIMER_LEDBLINK);
#endif
            break;

#if 0
        case SIG_TIMER_LEDBLINK:
            //*((unsigned char *) 0x00800110) |= 0x02;
            UART2_MCR |= 0x02;
            break;
#endif

        case SIG_TRANSPORT_EVENT:
        case SIG_TRANSPORT_DATA:
            transport_input(s);
            break;

        case SIG_UART_RX:
            if (uart2_rx_handler) {
                uart2_rx_handler(s->raw[2], &s->raw[3]);
            }
            break;

        //default:
        //    printf("unhandled signal %08x\n", s->sig_no);
        }

        OSE_free_buf(&s);
    }
}

