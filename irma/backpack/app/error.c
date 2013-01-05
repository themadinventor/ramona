/*
 * Ericsson Bluetooth Baseband Controller
 * Error handling
 *
 * (c) 2012 <fredrik@z80.se>
 */

#include "host/ose.h"
#include "host/app.h"
#include "uart.h"
#include "utils.h"

void div_by_zero(unsigned int dummy)
{
    UART2WriteString("\n\r\n\r### Division by Zero ###\n\r");
    UART2WriteString("Process halted.\n\r");

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

void panic(void)
{
    UART2WriteString("\n\r\n\r### OSE Crash ###\n\r");

    UART2WriteString("code = ");
    WriteHex(ose_crash_info.code);
    UART2WriteString("\n\rpid  = ");
    WriteHex(ose_crash_info.pid);
    UART2WriteString("\n\rpcb  = ");
    WriteHex(ose_crash_info.pcb);
    UART2WriteString("\n\rsp   = ");
    WriteHex(ose_crash_info.sp);
    UART2WriteString("\n\rdom  = ");
    WriteHex(ose_crash_info.domain);

#if 1
    UART2WriteString("\n\r\n\rSystem will reboot.\n\r");

    // Provoke watchdog to reset IRMA
    *((unsigned int *)0x00800c10) = 0x00;
    *((unsigned int *)0x00800c0c) = 0xc0;
    *((unsigned int *)0x00800c0c) = 0x18;

    for (;;) ;
#else
    UART2WriteString("\n\r\n\rSystem will hang.\n\r");
    for (;;) ;
#endif
}
