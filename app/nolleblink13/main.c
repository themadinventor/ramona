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

static const u8_t spp_service_record[] =
{
		SDP_DES_SIZE8, 0x8, 
			SDP_UINT16, 0x0, 0x0, /* Service record handle attribute */
				SDP_UINT32, 0x00, 0x00, 0x00, 0x00, /*dummy vals, filled in on xmit*/ 
		SDP_DES_SIZE8, 0x16, 
			SDP_UINT16, 0x0, 0x1, /* Service class ID list attribute */
			SDP_DES_SIZE8, 17,
				SDP_UUID128, 0x00, 0x00, 0x11, 0x01,
                    0x00, 0x00,
                    0x10, 0x00,
                    0x80, 0x00,
                    0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb,
		SDP_DES_SIZE8, 0x11,
			SDP_UINT16, 0x0, 0x4, /* Protocol descriptor list attribute */
			SDP_DES_SIZE8, 0xc, 
				SDP_DES_SIZE8, 0x3,
					SDP_UUID16, 0x1, 0x0, /*L2CAP*/
				SDP_DES_SIZE8, 0x5,
					SDP_UUID16, 0x0, 0x3, /*RFCOMM*/
					SDP_UINT8, 0x2, /*RFCOMM channel*/
		SDP_DES_SIZE8, 0x8,
			SDP_UINT16, 0x0, 0x5, /*Browse group list */
			SDP_DES_SIZE8, 0x3,
				SDP_UUID16, 0x10, 0x02, /*PublicBrowseGroup*/
};

struct sdp_record *sdp_record;
struct rfcomm_pcb *listener;
int nb_len;
int (*handler)(struct rfcomm_pcb *, uint8_t);

void delay(unsigned int ticks)
{
    unsigned int wake = OSE_get_ticks() + ticks;
    while (OSE_get_ticks() < wake) ;
}

void enter_bsl(void)
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

void reset(void)
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

int bsl_write(struct rfcomm_pcb *pcb, uint8_t c)
{
    if (nb_len == 0) {
        nb_len = c;
    } else {
        UART3PutChar(c);
        
        if (--nb_len == 0) {
            bt_rfcomm_write(pcb, "k", 1);
        }
    }
    
    return nb_len == 0;
}

int bsl_read(struct rfcomm_pcb *pcb, uint8_t c)
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

int msp_write(struct rfcomm_pcb *pcb, uint8_t c)
{
    if (nb_len == 0) {
        nb_len = c;
    } else {
        UART2PutChar(c);
        
        if (--nb_len == 0) {
            bt_rfcomm_write(pcb, "k", 1);
        }
    }
    
    return nb_len == 0;
}

int msp_read(struct rfcomm_pcb *pcb, uint8_t c)
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

inline static void nb_process_byte(struct rfcomm_pcb *pcb, uint8_t c)
{
    if (!handler) {
        // command
        nb_len = 0;
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

void nb_proc(struct rfcomm_pcb *pcb, int event, void *ptr, size_t len)
{
    if (event == RFCOMM_ACCEPTED) {
        handler = NULL;

    } else if (event == RFCOMM_RECEIVED) {
        char *c = ptr;
        while (len--) {
            nb_process_byte(pcb, *c++);
        }

    } else if (event == RFCOMM_DISCONNECTED) {
    }
}

void teardown(void)
{
    //printf("nb13: teardown\n");

    if (sdp_record != NULL) {
        //printf("spp: sdp unregister & free\n");
        sdp_unregister_service(sdp_record); 
        sdp_record_free(sdp_record);
    }

    if (listener != NULL) {
        rfcomm_close(listener);
        listener = NULL;
    }

    //printf("nb13: i'm outta here!\n");
}

int start(int p)
{
    //UART2SetBaudRate(UART_460800);
    //printf("nb13: start\n");
    
#if 0
    // Set UART3 TxD to float
    UART_GPIO |= UART3_TXD_FLOAT;

    // Set UART2 OUT1 (MSP430 Reset) high
    UART2_MCR &= ~0x02;
#endif

    // UART3 Clock?
    *((unsigned int *)0x0080090c) |= 0x40;

    // Reset MSP430
    reset();

    // Set UART3 baud to 9600 fo BSL
    UART3_BAUD = UART_BAUD_9600;

    UART2_BAUD = UART_BAUD_9600;

    // Set even parity on UART3 for BSL
    UART3_LCR |= 0x18;

    // Enable and reset FIFOs on UART3
    UART3_FCR = 0x03;

    plugin_teardown(teardown);

    if ((listener = bt_rfcomm_listen(2, nb_proc)) == NULL) {
        //printf("nb13: failed to allocate rfcomm socket\n");
        return -1;
    }

	if ((sdp_record = sdp_record_new((u8_t *)spp_service_record, sizeof(spp_service_record))) != NULL) {
		sdp_register_service(sdp_record);
	} else {
        //printf("nb13: failed to allocate sdp record\n");
        return -1;
    }

    //printf("nb13: initialization complete\n");

    return 0;
}

