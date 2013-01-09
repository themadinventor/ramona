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

void spp_proc(struct rfcomm_pcb *pcb, int event, void *ptr, size_t len)
{
    if (event == RFCOMM_ACCEPTED) {
        printf("\n*** Connected\n");
    } else if (event == RFCOMM_RECEIVED) {
        char *c = ptr;
        while (len--) {
            printf("%c", *c++);
        }
    } else if (event == RFCOMM_DISCONNECTED) {
        printf("\n*** Disconnected\n");
    }
}

void teardown(void)
{
    printf("spp: teardown\n");

    if (sdp_record != NULL) {
        printf("spp: sdp unregister & free\n");
        sdp_unregister_service(sdp_record); 
        sdp_record_free(sdp_record);
    }

    if (listener != NULL) {
        rfcomm_close(listener);
        listener = NULL;
    }

    printf("spp: i'm outta here!\n");
}

int start(int p)
{
    printf("spp: start\n");

    plugin_teardown(teardown);

    if ((listener = bt_rfcomm_listen(2, spp_proc)) == NULL) {
        return -1;
    }

	if ((sdp_record = sdp_record_new((u8_t *)spp_service_record, sizeof(spp_service_record))) != NULL) {
		sdp_register_service(sdp_record);
	} else {
        printf("spp: failed to allocate sdp record\n");
        return -1;
    }

    printf("spp: initialization complete\n");

    return 0;
}

