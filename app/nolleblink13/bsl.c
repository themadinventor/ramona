#include <stdio.h>
#include <stdint.h>

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

#include "utils/uart.h"

#include "host/app.h"
#include "host/irma.h"

#include "btstack.h"

#include "bsl.h"
#include "spp.h"

static struct sdp_record *bsl_sdp_record;
static struct rfcomm_pcb *bsl_listener;
static int bsl_len;
static int (*handler)(struct rfcomm_pcb *pcb, uint8_t);

static const u8_t bsl_service_record[] =
{
		SDP_DES_SIZE8, 0x8, 
			SDP_UINT16, 0x0, 0x0, /* Service record handle attribute */
				SDP_UINT32, 0x00, 0x00, 0x00, 0x00, /*dummy vals, filled in on xmit*/ 
		SDP_DES_SIZE8, 0x16, 
			SDP_UINT16, 0x0, 0x1, /* Service class ID list attribute */
			SDP_DES_SIZE8, 17,
				SDP_UUID128, 0x28, 0x25, 0xdf, 0x88,
                    0x6b, 0xfc,
                    0x43, 0xf6,
                    0xaf, 0x79,
                    0x0d, 0x08, 0x23, 0xc0, 0x95, 0x14,
		SDP_DES_SIZE8, 0x11,
			SDP_UINT16, 0x0, 0x4, /* Protocol descriptor list attribute */
			SDP_DES_SIZE8, 0xc, 
				SDP_DES_SIZE8, 0x3,
					SDP_UUID16, 0x1, 0x0, /*L2CAP*/
				SDP_DES_SIZE8, 0x5,
					SDP_UUID16, 0x0, 0x3, /*RFCOMM*/
					SDP_UINT8, 0x3, /*RFCOMM channel*/
		SDP_DES_SIZE8, 0x8,
			SDP_UINT16, 0x0, 0x5, /*Browse group list */
			SDP_DES_SIZE8, 0x3,
				SDP_UUID16, 0x10, 0x02, /*PublicBrowseGroup*/
};

static void delay(unsigned int ticks)
{
    unsigned int wake = OSE_get_ticks() + ticks;
    while (OSE_get_ticks() < wake) ;
}

static void enter_bsl(void)
{
    // Set UART2 TxD to float
    UART_GPIO |= UART2_TXD_FLOAT;

    // Set RST low
    UART2_MCR |= 0x02;
    
    // Set TEST low (UART3 Tx)
    UART_GPIO &= ~UART3_TXD_MASK;

    delay(1);
    
    // Set TEST high
    UART_GPIO |= UART3_TXD_HIGH;

    delay(1);

    // Set TEST low
    UART_GPIO &= ~UART3_TXD_HIGH;

    delay(1);

    // Set TEST high
    UART_GPIO |= UART3_TXD_HIGH;

    delay(1);
    
    // Set RST high
    UART2_MCR &= ~0x02;

    // Drive UART3 TxD
    UART_GPIO = (UART_GPIO & ~UART3_TXD_MASK) | UART3_TXD_TXD;

    // Flush UART3
    while (UART3GetRxFIFOSize() > 0) {
        UART3ReadByte();
    }
}

static void reset(void)
{
    // Set UART3 TxD (=TEST) to float
    UART_GPIO |= UART3_TXD_FLOAT;

    // Set RST low
    UART2_MCR |= 0x02;

    delay(1);

    // Set RST high
    UART2_MCR &= ~0x02;
    
    // Set UART2 TxD to drive
    UART_GPIO = (UART_GPIO & ~UART2_TXD_MASK) | UART2_TXD_TXD;
}

static int bsl_write(struct rfcomm_pcb *pcb, uint8_t c)
{
    if (bsl_len == 0) {
        bsl_len = c;
    } else {
        UART3PutChar(c);
        
        if (--bsl_len == 0) {
            bt_rfcomm_write(pcb, "k", 1);
        }
    }
    
    return bsl_len == 0;
}

static int bsl_read(struct rfcomm_pcb *pcb, uint8_t c)
{
    int read = 0;
    char buf[17];

    while (UART3GetRxFIFOSize() > 0 && read < c && read < 16) {
        buf[1+read++] = UART3ReadByte(); 
    }

    buf[0] = read;
    bt_rfcomm_write(pcb, buf, 1+read);

    return 1;
}

static int msp_write(struct rfcomm_pcb *pcb, uint8_t c)
{
    if (bsl_len == 0) {
        bsl_len = c;
    } else {
        UART2PutChar(c);
        
        if (--bsl_len == 0) {
            bt_rfcomm_write(pcb, "k", 1);
        }
    }
    
    return bsl_len == 0;
}

static int msp_read(struct rfcomm_pcb *pcb, uint8_t c)
{
    int read = 0;
    char buf[17];

    while (UART2GetRxFIFOSize() > 0 && read < c && read < 16) {
        buf[1+read++] = UART2ReadByte(); 
    }

    buf[0] = read;
    bt_rfcomm_write(pcb, buf, 1+read);

    return 1;
}

static int set_baud(struct rfcomm_pcb *pcb, uint8_t c)
{
    if (c == '?') {
        char buf = spp_get_baud();
        bt_rfcomm_write(pcb, &buf, 1);
        return 1;
    }

    if ((c <= 0x09) || ((c >= 0x10) || (c <= 0x19))) {
        spp_set_baud(c);
        bt_rfcomm_write(pcb, "k", 1);
    } else {
        bt_rfcomm_write(pcb, "E", 1);
    }
    return 1;
}

inline static void bsl_process_byte(struct rfcomm_pcb *pcb, uint8_t c)
{
    if (!handler) {
        // command
        bsl_len = 0;
        switch (c) {
            case 'v': // send version
                {
                    char *greeting = "Nolleblink 2013 (c) ElektroTekniska Föreningen.\n";
                    bt_rfcomm_write(pcb, greeting, strlen(greeting));
                    bt_rfcomm_write(pcb, bp_build_time, strlen(bp_build_time));
                    bt_rfcomm_write(pcb, "\n", 1);
                    bt_rfcomm_write(pcb, bp_build_comment, strlen(bp_build_comment));
                    bt_rfcomm_write(pcb, "\n", 1);
                }
                return;

            case 'b': // reset and enter BSL
                enter_bsl();
                bt_rfcomm_write(pcb, "k", 1);
                break;

            case 'B': // set SPP baudrate
                handler = set_baud;
                break;

            case 'x': // reset (leave BSL)
                reset();
                bt_rfcomm_write(pcb, "k", 1);
                break;

            case 'w': // write to bsl (length + data)
                handler = bsl_write;
                break;

            case 'r': // read from bsl (wanted length)
                handler = bsl_read;
                break;

            case 'W': // write to msp
                handler = msp_write;
                break;

            case 'R': // read from msp
                handler = msp_read;
                break;

            case 'f': // flash default firmware
                break;

            case 'q': // disconnect
                bt_rfcomm_disconnect(pcb);
                break;

            default:
                bt_rfcomm_write(pcb, "E", 1);
                bt_rfcomm_disconnect(pcb);
        }
    } else {
        if (handler(pcb, c)) {
            handler = NULL;
        }
    }
}

static void bsl_proc(struct rfcomm_pcb *pcb, int event, void *ptr, size_t len)
{
    if (event == RFCOMM_ACCEPTED) {
        handler = NULL;

    } else if (event == RFCOMM_RECEIVED) {
        char *c = ptr;
        while (len--) {
            bsl_process_byte(pcb, *c++);
        }

    } else if (event == RFCOMM_DISCONNECTED) {
    }
}

int bsl_init(void)
{
    // UART3 Clock?
    *((unsigned int *)0x0080090c) |= 0x40;

    // Reset MSP430
    reset();

    // Set UART3 baud to 9600 fo BSL
    UART3_BAUD = UART_BAUD_9600;
    
    // Set even parity on UART3 for BSL
    UART3_LCR |= 0x18;

    // Enable and reset FIFOs on UART3
    UART3_FCR = 0x03;

    if ((bsl_listener = bt_rfcomm_listen(3, bsl_proc)) == NULL) {
        return -1;
    }

	if ((bsl_sdp_record = sdp_record_new((u8_t *)bsl_service_record, sizeof(bsl_service_record))) != NULL) {
		sdp_register_service(bsl_sdp_record);
	} else {
        return -1;
    }
}

void bsl_teardown(void)
{
    if (bsl_sdp_record != NULL) {
        sdp_unregister_service(bsl_sdp_record); 
        sdp_record_free(bsl_sdp_record);
    }

    if (bsl_listener != NULL) {
        rfcomm_close(bsl_listener);
        bsl_listener = NULL;
    }
}

