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
            SDP_UUID128, 0x7b, 0x35, 0xf0, 0xbc,
                0x38, 0xf7,
                0x40, 0x5a,
                0x8d, 0x3c,
                0xa4, 0x45, 0x00, 0x6c, 0xed, 0x33,
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

void i2cm_proc(struct rfcomm_pcb *pcb, int event, void *ptr, size_t len)
{
    static uint8_t state, addr, reg, data;

    if (event == RFCOMM_ACCEPTED) {
        state = 0;
    } else if (event == RFCOMM_RECEIVED) {
        uint8_t *c = ptr;
        while (len--) {
            switch (state) {
            case 0:
                addr = 0x08;  *c++;
                reg = 0xee, data = 0xee;
                state++;
                break;

            case 1:
                reg = *c++;
                if (addr & 0x01) { // read
                    I2C_Read(addr, reg, &data);
                    bt_rfcomm_write(pcb, &data, 1);
                    state = 0;
                } else {
                    state++;
                }
                break;

            case 2:
                data = *c++;
                I2C_Write(addr, reg, data);
                state = 0;
                break;
            }
        }
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

    if ((listener = bt_rfcomm_listen(2, i2cm_proc)) == NULL) {
        printf("i2cm: failed to allocate rfcomm socket\n");
        return -1;
    }

	if ((sdp_record = sdp_record_new((u8_t *)spp_service_record, sizeof(spp_service_record))) != NULL) {
		sdp_register_service(sdp_record);
	} else {
        printf("i2cm: failed to allocate sdp record\n");
        return -1;
    }

    return 0;
}

