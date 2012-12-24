/*
 * Ericsson Bluetooth Baseband Controller
 * Backpack Main
 *
 * (c) 2012 <fredrik@z80.se>
 */

#include "host/ose.h"
#include "host/app.h"

#include "uart.h"
#include "transport.h"
#include "utils.h"
#include "signals.h"

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

/*
 * vanilla bss at 00000e74 and 54548 bytes long:
 * bss ends at 0000e388
 * 
 * our bss at 0000e400 until end of space and time (15 kB-ish)
 */

extern void _text, _etext, _data, _edata, _bss, _ebss;

void bpMain(void);

//extern void InitHW(void);
extern void bt_spp_start(void);
extern void bt_spp_tmr(void);
extern void spp_write(char *buf, int len);

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
 * This might be called by the runtime functions in math.S
 */
void __div0(void)
{
    UART2WriteString("OHMYGAAH! Division By Zero!\n");
    for (;;);
}

/*
 * IT'S A TRAP!
 */
void trap(unsigned int dummy)
{
    UART2WriteString("\n\r\n\r####### IT'S A TRAP! #######\n\r\n\r");

    unsigned int *ptr = &dummy;
    int i;
    for (i=0; i<32; i++, ptr++) {
        WriteHex((unsigned int)ptr);
        UART2WriteString(": ");
        WriteHex(*ptr);

        if (*ptr > 0x01000000 && *ptr < 0x010fffff) {
            UART2WriteString(" <-- ");
        }

        UART2WriteString("\n");
    }

    UART2WriteString("\n\r#### End of stack trace ####\n\r");
    for (;;) ;
}

/*
 * Initialize the data segment from flash and clear the .bss seg.
 */
void InitDataZeroBSS(void)
{
    register char *src = &_etext;
    register char *dst = &_data;
    register char *end = &_edata;

    while (dst < end)
        *dst++ = *src++;

    for (dst = &_bss, end = &_ebss; dst < end; dst++)
        *dst = 0;
}

/*
 * This gets called before OSE is initialized.
 * It must return zero.
 */
int init_hook(void)
{
    InitDataZeroBSS();

    // We must return 0, or we'll get unknown behaviour
    return 0;
}

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

int lwbt_init(void)
{
    //printf("Initializing lwBT...\n");
    mem_init();
    memp_init();
    pbuf_init();
    //printf("Memory management initialized\n");

    lwbt_memp_init();
    transport_init();
    if (hci_init() != ERR_OK) {
        printf("HCI initialization failed\n");
        return -1;
    }

    l2cap_init();
    sdp_init();
    rfcomm_init();
    //printf("lwBT initialized\n");
    return 0;
}

void lwbt_timer(void)
{
    //unsigned int tick = OSE_get_ticks();

    //printf("\x1b[s\x1b[H\x1b[44m{ NolleBT %08x }\x1b[0m\x1b[u", tick);

    //printf("lwbt_timer\n");
    
	l2cap_tmr();
	rfcomm_tmr();
	bt_spp_tmr();

    static int inquiry_timeout = 60;
    if ((inquiry_timeout) && (!(--inquiry_timeout))) {
        hci_write_scan_enable(HCI_SCAN_EN_PAGE);
    }
}

void nolle_transmit(unsigned char type, void *data, unsigned char len)
{
#if 0
    UART2PutChar(0xa5);
    UART2PutChar(type);
    UART2PutChar(len);
    
    unsigned char *ptr = data;
    while (len--) {
        UART2PutChar(*ptr++);
    }
#else
    printf("%02x %d: ", type, len);
    unsigned char *ptr = data;
    while (len--) {
        UART2PutChar(*ptr++);
    }
    UART2PutChar('\n');
    UART2PutChar('\r');
#endif
}

void nolle_receive(unsigned char *data, unsigned char inlen)
{
    static unsigned char buf[256], idx = 0;

    while (inlen--) {
        buf[idx++] = *data++;

        if (idx == 1 && buf[0] != 0xa5) {
            idx = 0;
            continue;
        }

        if ((idx >= 3) && (buf[2] == idx-3)) {
            switch (buf[1]) {
            case 0x02:
                spp_disconnect();
                break;

            case 0x03:
                spp_write(&buf[3], buf[2]);
                break;

            case 0x04:
                hci_change_local_name(&buf[3], buf[2]);
                break;

            case 0x05:
                hci_write_scan_enable((buf[2] ? HCI_SCAN_EN_INQUIRY : 0) | HCI_SCAN_EN_PAGE);
                break;
            }

            idx = 0;
        }
    }
}

/*
 * This is the entry point of our thread
 */
void bpMain(void)
{
    //unsigned int sp;
    //asm ("mov %0, sp":"=r" (sp));

    //UART2SetBaudRate(UART_115200);
    //UART1SetBaudRate(UART_9600);

    //InitHW();
    //*((unsigned short *)0x00800d04) = 0xffff;
    //UARTInit();
    {
        unsigned char buf[64];
        sprintf(buf, "IRMA Backpack, Build: %s %s", build_time, build_comment);
        nolle_transmit(0x00, buf, strlen(buf));
    }


    //UART2WriteString("Uart 2!\n\r");
    //UART3SetBaudRate(UART_9600);
    //UART3WriteString("Uart 3! wej wej wej wej wej wej wej\n\r");
    
    //UART3PutChar('k');
    //UART3PutChar('o');
    //UART3PutChar('n');
    //UART3PutChar('g');
    //UART3PutChar('o');
    //UART2WriteString("Uart 2 again!\n\r");
    //UART1SetBaudRate(UART_9600);
    //UART1WriteString("*UART1*\n\r");
    //printf("Wrote to UART1\n");

    //UART3SetBaudRate(UART_9600);
    //UART3WriteString("*UART3*\n\r");
    //printf("Wrote to UART3\n");

    //for(;;);

    //printf("My stack is at %08x\n", sp);
    
    lwbt_init();

    bt_spp_start();

    {
        printf("I2C Test:\n\r");
        I2C_Init();
        
        //printf("Read: ");

        unsigned char res; //, data;
        //res = I2C_Read(0x02, 0xAA, &data);

        printf("Write: ");
        res = I2C_Write(0x02, 0xAA, 0xBB);

        printf("%02x\n\r", res);
    }

    //printf("Application started\n");

    timer_add(1000, SIG_TIMER);

    //printf("We're going in...\n\n");

    static const SIGSELECT anysig[] = {0};
    for(;;) {
        SIGNAL *s = OSE_receive((SIGSELECT *) anysig);

        switch (s->sig_no) {
        case SIG_TIMER:
            lwbt_timer();
            timer_add(1000, SIG_TIMER);
            break;

        case SIG_TRANSPORT_EVENT:
        case SIG_TRANSPORT_DATA:
            transport_input(s);
            break;

        case SIG_UART_RX:
            //spp_write(&s->raw[3], s->raw[2]);
            nolle_receive(&s->raw[3], s->raw[2]);
            break;

        default:
            printf("unhandled signal %08x\n", s->sig_no);
        }

        OSE_free_buf(&s);
    }
}

