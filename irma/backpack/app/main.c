/*
 * Ericsson Bluetooth Baseband Controller
 * Backpack Main
 *
 * (c) 2012 <fredrik@z80.se>
 */

#include "host/ose.h"
#include "host/app.h"

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
    .ptr1 = (void *)0x73d0, // linked list of processes with given prio? wrong (=other) value will crash OSE.
    .pcb = procmem.bpPCB,
    .ptr2 = procmem.stack+sizeof(procmem.stack), // top of stack?
    .ptr3 = procmem.stack, // bottom of stack?
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


/*
 * This is the entry point of our thread
 */
void bpMain(void)
{
    lwbt_init();
    monitor_init();

    plugin_enable();

    //I2C_Init();

    timer_add(1000, SIG_TIMER);

    static const SIGSELECT anysig[] = {0};
    for(;;) {
        SIGNAL *s = OSE_receive((SIGSELECT *) anysig);

        switch (s->sig_no) {
        case SIG_TIMER:
            //UART2PutChar(0x03);
            
            //I2C_Write(2, 1, 3);

            lwbt_timer();
            timer_add(1000, SIG_TIMER);
            break;

        case SIG_TRANSPORT_EVENT:
        case SIG_TRANSPORT_DATA:
            transport_input(s);
            break;

        case SIG_UART_RX:
            //spp_write(&s->raw[3], s->raw[2]);
            //nolle_receive(&s->raw[3], s->raw[2]);
            break;

        default:
            printf("unhandled signal %08x\n", s->sig_no);
        }

        OSE_free_buf(&s);
    }
}

