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

#include "btstack.h"
#include "host/app.h"

static const u8_t spp_service_record[] =
{
		SDP_DES_SIZE8, 0x8, 
			SDP_UINT16, 0x0, 0x0, /* Service record handle attribute */
				SDP_UINT32, 0x00, 0x00, 0x00, 0x00, /*dummy vals, filled in on xmit*/ 
		SDP_DES_SIZE8, 0x16, 
			SDP_UINT16, 0x0, 0x1, /* Service class ID list attribute */
			SDP_DES_SIZE8, 17,
				SDP_UUID128, 0xf7, 0x5a, 0xa1, 0x7a,
                    0xe9, 0xdc,
                    0x4f, 0xd4,
                    0xbb, 0x79,
                    0x06, 0xff, 0x4e, 0x66, 0x3a, 0xa4,
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

inline static void pyio_process(struct rfcomm_pcb *pcb, uint8_t c)
{
    static uint32_t state, op;

    if (!state) {
        if (c & 0x80) {
            // op is a write, wait for data
            op = c & 0x7f;
            state = 1;
        } else {
            // op is a read. do it.
            uint8_t data;
            I2C_Read(2, c, &data);
            //printf("pyio: read %02x -> %02x\n", op, data);
            bt_rfcomm_write(pcb, &data, 1);
        }
    } else {
        // op is a write, and we've got the data now.
        I2C_Write(2, op, c);
        //printf("pyio: write %02x <- %02x\n", op, c);
        state = 0;
    }
}

void pyio_proc(struct rfcomm_pcb *pcb, int event, void *ptr, size_t len)
{
    if (event == RFCOMM_ACCEPTED) {
        //printf("pyio: Connected\n");
    } else if (event == RFCOMM_RECEIVED) {
        char *c = ptr;
        while (len--) {
            //printf("pyio: recv %02x\n", *c);
            pyio_process(pcb, *c++);
        }
    } else if (event == RFCOMM_DISCONNECTED) {
        //printf("pyio: Disconnected\n");
    }
}

void teardown(void)
{
    if (sdp_record != NULL) {
        sdp_unregister_service(sdp_record); 
        sdp_record_free(sdp_record);
    }

    if (listener != NULL) {
        rfcomm_close(listener);
        listener = NULL;
    }
}

int start(int p)
{
    plugin_teardown(teardown);

    I2C_Init();

    if ((listener = bt_rfcomm_listen(2, pyio_proc)) == NULL) {
        printf("pyio: failed to allocate rfcomm socket\n");
        return -1;
    }

	if ((sdp_record = sdp_record_new((u8_t *)spp_service_record, sizeof(spp_service_record))) != NULL) {
		sdp_register_service(sdp_record);
	} else {
        printf("pyio: failed to allocate sdp record\n");
        return -1;
    }

    return 0;
}

