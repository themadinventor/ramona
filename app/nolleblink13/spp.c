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

#include "spp.h"
#include "files.h"

static const u8_t spp_service_record[] =
{
		SDP_DES_SIZE8, 0x8, 
			SDP_UINT16, 0x0, 0x0, /* Service record handle attribute */
				SDP_UINT32, 0x00, 0x00, 0x00, 0x00, /*dummy vals, filled in on xmit*/ 
		SDP_DES_SIZE8, 0x16, 
			SDP_UINT16, 0x0, 0x1, /* Service class ID list attribute */
			SDP_DES_SIZE8, 17,
				SDP_UUID128, 0x53, 0xc6, 0x0a, 0x67,
                    0xb9, 0x1d,
                    0x48, 0x8d,
                    0xa0, 0xc6,
                    0xb7, 0x0c, 0x75, 0x2a, 0x74, 0x47,
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

static struct sdp_record *spp_sdp_record;
static struct rfcomm_pcb *spp_listener, *spp_conn;

static void spp_proc(struct rfcomm_pcb *pcb, int event, void *ptr, size_t len)
{
    if (event == RFCOMM_ACCEPTED) {
        spp_conn = pcb;
    } else if (event == RFCOMM_RECEIVED) {
        char *c = ptr;
        while (len--) {
            UART2PutChar(*c++);
        }

    } else if (event == RFCOMM_DISCONNECTED) {
        spp_conn = NULL;
    }
}

static void spp_uart_rx(int bytes, char *data)
{
    bt_rfcomm_write(spp_conn, data, bytes);
}

void spp_set_baud(unsigned char baud)
{
    NVDS_WriteFile(NVDS_SPP_BAUD, 1, &baud);
    UART2_BAUD = baud;
}

unsigned char spp_get_baud(void)
{
    return UART2_BAUD;
}

int spp_init(void)
{
    if ((spp_listener = bt_rfcomm_listen(2, spp_proc)) == NULL) {
        return -1;
    }

	if ((spp_sdp_record = sdp_record_new((u8_t *)spp_service_record, sizeof(spp_service_record))) != NULL) {
		sdp_register_service(spp_sdp_record);
	} else {
        return -1;
    }

    set_uart2_rx_handler(spp_uart_rx);

    unsigned char baud;
    if (NVDS_ReadFile(NVDS_SPP_BAUD, 1, &baud)) {
        UART2_BAUD = baud;
    } else {
        UART2_BAUD = UART_BAUD_9600;
    }
}

void spp_teardown(void)
{
    set_uart2_rx_handler(NULL);

    if (spp_sdp_record != NULL) {
        sdp_unregister_service(spp_sdp_record); 
        sdp_record_free(spp_sdp_record);
    }

    if (spp_listener != NULL) {
        rfcomm_close(spp_listener);
        spp_listener = NULL;
    }
}

