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

#define HID_CTRL_PSM 0x11
#define HID_INTR_PSM 0x13

static const u8_t hid_service_record[] = {
    //SDP_DES_SIZE16, 0x01, 0x7a,
        SDP_UINT16, 0x00, 0x01,
            SDP_DES_SIZE8,  0x03,
                SDP_UUID16, 0x11, 0x24,
        SDP_UINT16, 0x00, 0x04,
            SDP_DES_SIZE8, 0x0c, 
                SDP_DES_SIZE8, 0x05,
                    SDP_UUID16, 0x01, 0x00, 
                    SDP_UINT8, 0x11,
                SDP_DES_SIZE8, 0x03,
                    SDP_UUID16, 0x00, 0x11,
        SDP_UINT16, 0x00, 0x05,
            SDP_DES_SIZE8, 0x03,
                SDP_UUID16, 0x10, 0x02,
        SDP_UINT16, 0x00, 0x06,
            SDP_DES_SIZE8, 0x09,
                SDP_UINT16, 0x65, 0x6e,
                SDP_UINT16, 0x00, 0x6a,
                SDP_UINT16, 0x01, 0x00,
        SDP_UINT16, 0x00, 0x09,
            SDP_DES_SIZE8, 0x08,
                SDP_DES_SIZE8, 0x06,
                    SDP_UUID16, 0x11, 0x24,
                    SDP_UINT16, 0x01, 0x00,
        SDP_UINT16, 0x00, 0x0d,
            SDP_DES_SIZE8, 0x0e,
                SDP_DES_SIZE8, 0x0c,
                    SDP_DES_SIZE8, 0x05,
                        SDP_UUID16, 0x01, 0x00,
                        SDP_UINT8, 0x13,
                    SDP_DES_SIZE8, 0x03,
                        SDP_UUID16, 0x00, 0x11,
        /*SDP_UINT16, 0x01, 0x00,
            SDP_TEXT_STR8, 0x17, 0x46, 0x61, 0x6b, 0x65, 0x20, 0x42, 0x6c, 0x75, 0x65, 
                0x74, 0x6f, 0x6f, 0x74, 0x68, 0x20, 0x4b, 0x65, 0x79, 0x62, 0x6f, 0x61, 0x72, 0x64,
        SDP_UINT16, 0x01, 0x01,
            SDP_TEXT_STR8, 0x42, 0x50, 0x72, 0x6f, 0x76, 0x69, 0x64, 0x65, 0x73, 0x20, 0x61, 0x20, 0x73, 0x69, 
                0x6d, 0x70, 0x6c, 0x65, 0x20, 0x48, 0x49, 0x44, 0x20, 0x64, 0x65, 0x76, 0x69, 0x63, 0x65, 0x20, 
                0x6f, 0x76, 0x65, 0x72, 0x20, 0x42, 0x6c, 0x75, 0x65, 0x74, 0x6f, 0x6f, 0x74, 0x68, 0x20, 0x66, 
                0x6f, 0x72, 0x20, 0x79, 0x6f, 0x75, 0x72, 0x20, 0x6d, 0x6f, 0x62, 0x69, 0x6c, 0x65, 0x20, 0x64, 
                0x65, 0x76, 0x69, 0x63, 0x65, 
        SDP_UINT16, 0x01, 0x02,
            SDP_TEXT_STR8, 0x00,*/
        SDP_UINT16, 0x02, 0x00,
            SDP_UINT16, 0x01, 0x00, 
        SDP_UINT16, 0x02, 0x01,
            SDP_UINT16, 0x01, 0x11,
        SDP_UINT16, 0x02, 0x02,
            SDP_UINT16, 0x00, 0x40,
        SDP_UINT16, 0x02, 0x03,
            SDP_UINT16, 0x00, 0x0d,
        SDP_UINT16, 0x02, 0x04,
            SDP_UINT16, 0x00, 0x01,
        SDP_UINT16, 0x02, 0x05,
            SDP_UINT16, 0x00, 0x01,
        SDP_UINT16, 0x02, 0x06,
            SDP_DES_SIZE8, 0x69,
                SDP_DES_SIZE8, 0x67,
                    0x08, 0x22,
                    SDP_TEXT_STR8, 0x63,
                        0x05, 0x01, 0x09, 0x06, 0xa1, 0x01, 0x85, 
                        0x01, 0x05, 0x07, 0x19, 0xe0, 0x29, 0xe7, 0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x08, 0x81, 
                        0x02, 0x75, 0x08, 0x95, 0x01, 0x81, 0x01, 0x75, 0x01, 0x95, 0x05, 0x05, 0x08, 0x19, 0x01, 0x29, 
                        0x05, 0x91, 0x02, 0x75, 0x03, 0x95, 0x01, 0x91, 0x01, 0x75, 0x08, 0x95, 0x06, 0x15, 0x00, 0x26, 
                        0xff, 0x00, 0x05, 0x07, 0x19, 0x00, 0x2a, 0xff, 0x00, 0x81, 0x00, 0x75, 0x01, 0x95, 0x01, 0x15, 
                        0x00, 0x25, 0x01, 0x05, 0x0c, 0x09, 0xb8, 0x81, 0x06, 0x09, 0xe2, 0x81, 0x06, 0x09, 0xe9, 0x81, 
                        0x02, 0x09, 0xea, 0x81, 0x02, 0x75, 0x01, 0x95, 0x04, 0x81, 0x01, 0xc0,
        SDP_UINT16, 0x02, 0x07,
            SDP_DES_SIZE8, 0x08,
                SDP_DES_SIZE8, 0x06,
                    SDP_UINT16, 0x04, 0x09,
                    SDP_UINT16, 0x01, 0x00,
        SDP_UINT16, 0x02, 0x08,
            SDP_UINT16, 0x00, 0x00,
        SDP_UINT16, 0x02, 0x0a,
            SDP_UINT16, 0x00, 0x01,
        SDP_UINT16, 0x02, 0x0b,
            SDP_UINT16, 0x01, 0x00,
        SDP_UINT16, 0x02, 0x0c,
            SDP_UINT16, 0x1f, 0x40,
        SDP_UINT16, 0x02, 0x0d,
            SDP_UINT16, 0x00, 0x01,
        SDP_UINT16, 0x02, 0x0e,
            SDP_UINT16, 0x00, 0x01,
};

#if 0
static const u8_t hid_service_record[] =
{
		SDP_DES_SIZE8, 0x8, 
			SDP_UINT16, 0x0, 0x0, /* Service record handle attribute */
				SDP_UINT32, 0x00, 0x00, 0x00, 0x00, /*dummy vals, filled in on xmit*/ 
		SDP_DES_SIZE8, 0x8, 
			SDP_UINT16, 0x0, 0x1, /* Service class ID list attribute */
			SDP_DES_SIZE8, 0x03,
				SDP_UUID16, 0x11, 0x24,
		SDP_DES_SIZE8, 0x0f,
			SDP_UINT16, 0x0, 0x4, /* Protocol descriptor list attribute */
			SDP_DES_SIZE8, 0xa, 
				SDP_DES_SIZE8, 0x3,
					SDP_UUID16, 0x1, 0x00, /*L2CAP*/
				SDP_DES_SIZE8, 0x3,
					SDP_UUID16, 0x0, 0x11, /*HIDP*/
		SDP_DES_SIZE8, 0x8,
			SDP_UINT16, 0x0, 0x5, /*Browse group list */
			SDP_DES_SIZE8, 0x3,
				SDP_UUID16, 0x10, 0x02, /*PublicBrowseGroup*/
        SDP_DES_SIZE8, 110,
            SDP_UINT16, 0x02, 0x6,
            SDP_DES_SIZE8, 105,
                SDP_DES_SIZE8, 2,
                    0x08, 0x22,
                SDP_DES_SIZE8, 99,
                    0x05, 0x01, // usage page
        0x09, 0x06, // keyboard
        0xa1, 0x01, // key codes
        0x85, 0x01, // minimum
        0x05, 0x07, // max
        0x19, 0xe0, // logical min
        0x29, 0xe7, // logical max
        0x15, 0x00, // report size
        0x25, 0x01, // report count
        0x75, 0x01, // input data variable absolute
        0x95, 0x08, // report count
        0x81, 0x02, // report size
        0x75, 0x08, 
        0x95, 0x01, 
        0x81, 0x01, 
        0x75, 0x01, 
        0x95, 0x05,
        0x05, 0x08,
        0x19, 0x01,
        0x29, 0x05, 
        0x91, 0x02,
        0x75, 0x03,
        0x95, 0x01,
        0x91, 0x01,
        0x75, 0x08,
        0x95, 0x06,
        0x15, 0x00,
        0x26, 0xff,
        0x00, 0x05,
        0x07, 0x19,
        0x00, 0x2a,
        0xff, 0x00,
        0x81, 0x00,
        0x75, 0x01,
        0x95, 0x01,
        0x15, 0x00,
        0x25, 0x01,
        0x05, 0x0c,
        0x09, 0xb8,
        0x81, 0x06,
        0x09, 0xe2,
        0x81, 0x06,
        0x09, 0xe9,
        0x81, 0x02,
        0x09, 0xea,
        0x81, 0x02,
        0x75, 0x01,
        0x95, 0x04,
        0x81, 0x01,
        0xc0
};
#endif

struct sdp_record *sdp_record;
struct l2cap_pcb *hid_ctrl_listen, *hid_intr_listen, *hid_intr;

err_t hid_recv(void *arg, struct l2cap_pcb *pcb, struct pbuf *p, err_t err)
{
    printf("hid: recv on psm %02x\n", pcb->psm);
    return ERR_OK;
}

err_t hid_disconnect_ind(void *arg, struct l2cap_pcb *pcb, err_t err)
{
    printf("hid: disconnect psm %02x\n", pcb->psm);

    if (pcb->psm == HID_INTR_PSM) {
        hid_intr = NULL;
    }

    return ERR_OK;
}

err_t hid_connect_ind(void *arg, struct l2cap_pcb *pcb, err_t err)
{
    printf("hid: connect on psm %02x\n", pcb->psm);
    l2cap_disconnect_ind(pcb, hid_disconnect_ind);
    l2cap_recv(pcb, hid_recv);

    if (pcb->psm == HID_INTR_PSM) {
        hid_intr = pcb;
    }

    return ERR_OK;
}

err_t hid_write(struct l2cap_pcb *pcb, const char *buf, int len)
{
    struct pbuf *p = pbuf_alloc(PBUF_RAW, len, PBUF_RAM);
    memcpy(p->payload, buf, len);
    
    if (l2ca_datawrite(pcb, p) != ERR_OK) {
        printf("hid: failed to write\n");
    }

    pbuf_free(p);
}

void hid_key_down(int modifiers, int keycode)
{
    char buf[10] = {0xa1, 0x01, modifiers, 0x00, keycode, 0x00, 0x00, 0x00, 0x00, 0x00};

    if (hid_intr != NULL) {
        hid_write(hid_intr, buf, sizeof(buf));
    } else {
        printf("hid: key_down but no hid_intr pcb\n");
    }
}

void hid_key_up(int modifiers, int keycode)
{
    char buf[10] = {0xa1, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    if (hid_intr != NULL) {
        hid_write(hid_intr, buf, sizeof(buf));
    } else {
        printf("hid: key_up but no hid_intr pcb\n");
    }
}

void teardown(void)
{
    printf("hid: teardown\n");

    if (sdp_record != NULL) {
        printf("hid: sdp unregister & free\n");
        sdp_unregister_service(sdp_record); 
        sdp_record_free(sdp_record);
    }
    
    if (hid_ctrl_listen != NULL) {
        l2cap_close(hid_ctrl_listen);
    }

    if (hid_intr_listen != NULL) {
        l2cap_close(hid_intr_listen);
    }

    printf("hid: i'm outta here!\n");
}

int start(int p)
{
    printf("hid: start\n");

    plugin_teardown(teardown);

    if ((hid_ctrl_listen = l2cap_new()) != NULL) { 
        l2cap_connect_ind(hid_ctrl_listen, HID_CTRL_PSM, hid_connect_ind);
    } else {
        printf("hid: failed to allocate l2cap pcb\n");
        return -1;
    }

    if ((hid_intr_listen = l2cap_new()) != NULL) {
        l2cap_connect_ind(hid_intr_listen, HID_INTR_PSM, hid_connect_ind);
    } else {
        printf("hid: failed to allocate l2cap pcb\n");
        return -1;
    }

	if ((sdp_record = sdp_record_new((u8_t *)hid_service_record, sizeof(hid_service_record))) != NULL) {
		sdp_register_service(sdp_record);
	} else {
        printf("hid: failed to allocate sdp record\n");
        return -1;
    }

    printf("hid: initialization complete\n");

    return 0;
}

