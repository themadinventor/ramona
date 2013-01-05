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
#include "btstack.h"
#include "transport.h"

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

void monitor_greeting(struct rfcomm_pcb *pcb)
{
    char buf[64];
    sprintf(buf, "Ramona Monitor\nIRMA Build: %s, %s\n\r",
            build_time, build_comment);
    spp_write(pcb, buf, strlen(buf));

    const char *addr = get_bdaddr();
    sprintf(buf, "BDADDR: %02x:%02x:%02x:%02x:%02x:%02x\n\r\n\r",
		    addr[5], addr[4], addr[3],
			addr[2], addr[1], addr[0]);
    spp_write(pcb, buf, strlen(buf));
}

/*
 * This is the entry point of our thread
 */
void bpMain(void)
{
    //printf("Ramona r.1 / IRMA build %s %s\n", build_time, build_comment);

    /*
    printf("NVDS:\n");
    printf("  write = %08x, erase = %08x\n",
            NVDS_Context.write, NVDS_Context.erase);
    printf("  initd = %d, current_page = %d\n",
            NVDS_Context.initialized, NVDS_Context.current_page);
    printf("  tag = %08x, blob = %08x\n",
            NVDS_Context.tag_head, NVDS_Context.blob_head);
    printf("  free = %d, page = %d\n",
            NVDS_Context.free_space, NVDS_Context.page_size);
    printf("  pages at %08x and %08x\n",
            NVDS_Context.pages[0], NVDS_Context.pages[1]);
*/

    /*printf("Writing to flash...");
    char *str = "Testar flash, yeah!";
    Flash_Write(0x01056000, str, 20);
    printf("done\n");

    printf("Now, it says %s\n", 0x01056000);

    printf("Erasing flash...");
    Flash_ErasePage(0x01056000);
    printf("done\n");
    printf("Now, it's %02x\n", *((unsigned char *)0x01056000));
    */

    lwbt_init();

    timer_add(1000, SIG_TIMER);

    static const SIGSELECT anysig[] = {0};
    for(;;) {
        SIGNAL *s = OSE_receive((SIGSELECT *) anysig);

        switch (s->sig_no) {
        case SIG_TIMER:
            UART2PutChar(0x03);
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

        case SIG_BT_ACCEPTED:
            {
                struct sig_bt_accepted *p = (void *) s;
                printf("Accepted connection to cn=%d\n", rfcomm_cn(p->pcb));
                monitor_greeting(p->pcb);
            }
            break;

        case SIG_BT_DISCONNECTED:
            {
                struct sig_bt_disconnected *p = (void *) s;
                printf("Disconnected from cn=%d\n", rfcomm_cn(p->pcb));
            }
            break;

        case SIG_BT_RECEIVED:
            {
                struct sig_bt_received *p = (void *) s;
                char *c = p->data;
                printf("Received %d bytes on cn=%d\n: ", p->len, rfcomm_cn(p->pcb));
                while (p->len--) {
                    printf("%c", *c++);
                }
                printf("\n");
            }
            break;

        default:
            printf("unhandled signal %08x\n", s->sig_no);
        }

        OSE_free_buf(&s);
    }
}

