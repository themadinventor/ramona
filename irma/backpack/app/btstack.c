#include "btstack.h"

#include "host/ose.h"
#include "host/app.h"
#include "transport.h"

#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/stats.h"
#include "lwip/inet.h"

#include "lwbt/phybusif.h"
#include "lwbt/lwbt_memp.h"
#include "lwbt/hci.h"
#include "lwbt/l2cap.h"
#include "lwbt/sdp.h"
#include "lwbt/rfcomm.h"

#define BT_SPP_DEBUG LWIP_DBG_OFF /* Controls debug messages */

static const u8_t ramona_service_record[] =
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
                    /*0x00, 0x00, 0x00, 0x00,
					0xde, 0xca,
					0xfa, 0xde,
					0xde, 0xca,
					0xde, 0xaf, 0xde, 0xca, 0xca, 0xff,*/
		SDP_DES_SIZE8, 0x11,
			SDP_UINT16, 0x0, 0x4, /* Protocol descriptor list attribute */
			SDP_DES_SIZE8, 0xc, 
				SDP_DES_SIZE8, 0x3,
					SDP_UUID16, 0x1, 0x0, /*L2CAP*/
				SDP_DES_SIZE8, 0x5,
					SDP_UUID16, 0x0, 0x3, /*RFCOMM*/
					SDP_UINT8, 0x1, /*RFCOMM channel*/
		SDP_DES_SIZE8, 0x8,
			SDP_UINT16, 0x0, 0x5, /*Browse group list */
			SDP_DES_SIZE8, 0x3,
				SDP_UUID16, 0x10, 0x02, /*PublicBrowseGroup*/
};

struct rfcomm_pcb *spp_pcb;

struct bt_state {
	struct bd_addr bdaddr;
	struct pbuf *p;
	u8_t btctrl;
	u8_t cn;
} bt_spp_state;

err_t pin_req(void *arg, struct bd_addr *bdaddr)
{
	u8_t pin[] = "1234";
	LWIP_DEBUGF(BT_SPP_DEBUG, ("pin_req\n"));
	return hci_pin_code_request_reply(bdaddr, 4, pin);
}

err_t spp_recv(void *arg, struct rfcomm_pcb *pcb, struct pbuf *p, err_t err)
{
	struct pbuf *q = NULL;
	
	LWIP_DEBUGF(BT_SPP_DEBUG, ("spp_recv: p->len == %d p->tot_len == %d\n", p->len, p->tot_len));

	for (q = p; q != NULL; q = q->next) {
        //nolle_transmit(0x03, (u8_t*)q->payload, q->len);
	}
	
    pbuf_free(p);
	
	return ERR_OK;
}

void spp_write(char *buf, int len)
{
    if (!spp_pcb) {
        return;
    }

	struct pbuf *q = pbuf_alloc(PBUF_RAW, len, PBUF_RAM);
    memcpy(q->payload, buf, len);

	if (rfcomm_cl(spp_pcb)) {
        rfcomm_uih_credits(spp_pcb, PBUF_POOL_SIZE - rfcomm_remote_credits(spp_pcb), q);
	} else {
		rfcomm_uih(spp_pcb, rfcomm_cn(spp_pcb), q);
	}
	pbuf_free(q);
}

void spp_disconnect(void)
{
    if (!spp_pcb) {
        return;
    }

    rfcomm_disconnect(spp_pcb);
}

err_t rfcomm_disconnected(void *arg, struct rfcomm_pcb *pcb, err_t err) 
{
	err_t ret = ERR_OK;

    spp_pcb = NULL;

    //printf("Disconnected, cl=%02x, %x\n", pcb->cl, rfcomm_cl(pcb));
    //nolle_transmit(0x02, NULL, 0);

	LWIP_DEBUGF(BT_SPP_DEBUG, ("rfcomm_disconnected: CN = %d\n", rfcomm_cn(pcb)));
	if(rfcomm_cn(pcb) != 0) {
		; //ppp_lp_disconnected(pcb);
	}
	rfcomm_close(pcb);

	return ret;
}

err_t rfcomm_accept(void *arg, struct rfcomm_pcb *pcb, err_t err) 
{
	LWIP_DEBUGF(BT_SPP_DEBUG, ("rfcomm_accept: CN = %d\n", rfcomm_cn(pcb)));

	rfcomm_disc(pcb, rfcomm_disconnected);
	if(pcb->cn != 0) {
		//set recv callback
		rfcomm_recv(pcb, spp_recv);

        //printf("Incoming connection, cl=%02x, %x\n", pcb->cl, rfcomm_cl(pcb));
        /*{
            unsigned char buf[32];
            unsigned char *addr = pcb->l2cappcb->remote_bdaddr.addr;
            sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", addr[5], addr[4], addr[3],
                    addr[2], addr[1], addr[0]);
            nolle_transmit(0x01, buf, strlen(buf));
        }*/

	    /*struct pbuf *q = NULL;
        char str[] = "IRMA backpack säger hej!\n";
	    q = pbuf_alloc(PBUF_RAW, strlen(str), PBUF_RAM);
        memcpy(q->payload, str, strlen(str));*/

	    //if (rfcomm_cl(pcb)) {
		//    rfcomm_uih_credits(pcb, PBUF_POOL_SIZE - rfcomm_remote_credits(pcb), q);
	    //} else {
		//    rfcomm_uih(pcb, rfcomm_cn(pcb), q);
	    //}
	    //pbuf_free(q);
	}

    spp_pcb = pcb;

	return ERR_OK;
}

static err_t bt_disconnect_ind(void *arg, struct l2cap_pcb *pcb, err_t err)
{
	err_t ret;

	LWIP_DEBUGF(BT_SPP_DEBUG, ("bt_disconnect_ind\n"));

	if(pcb->psm == SDP_PSM) { 
		sdp_lp_disconnected(pcb);
	} else if(pcb->psm == RFCOMM_PSM) {
		ret = rfcomm_lp_disconnected(pcb);
	}

	l2cap_close(pcb);
	return ERR_OK;
}

err_t bt_connect_ind(void *arg, struct l2cap_pcb *pcb, err_t err)
{
	LWIP_DEBUGF(BT_SPP_DEBUG, ("bt_connect_ind\n"));

	/* Tell L2CAP that we wish to be informed of a disconnection request */
	l2cap_disconnect_ind(pcb, bt_disconnect_ind);

	/* Tell L2CAP that we wish to be informed of incoming data */
	if(pcb->psm == SDP_PSM) {
		l2cap_recv(pcb, sdp_recv);
	} else if (pcb->psm == RFCOMM_PSM) {
		l2cap_recv(pcb, rfcomm_input);
	}
	return ERR_OK;  
}

err_t bt_spp_init(void)
{
	struct l2cap_pcb *l2cappcb;
	struct rfcomm_pcb *rfcommpcb;
	struct sdp_record *record;

	if((l2cappcb = l2cap_new()) == NULL) {
		LWIP_DEBUGF(BT_SPP_DEBUG, ("bt_spp_init: Could not alloc L2CAP PCB for SDP_PSM\n"));
		return ERR_MEM;
	}
	l2cap_connect_ind(l2cappcb, SDP_PSM, bt_connect_ind);

	if((l2cappcb = l2cap_new()) == NULL) {
		LWIP_DEBUGF(BT_SPP_DEBUG, ("bt_spp_init: Could not alloc L2CAP PCB for RFCOMM_PSM\n"));
		return ERR_MEM;
	}
	l2cap_connect_ind(l2cappcb, RFCOMM_PSM, bt_connect_ind);

	LWIP_DEBUGF(RFCOMM_DEBUG, ("bt_spp_init: Allocate RFCOMM PCB for CN 0\n"));
	if((rfcommpcb = rfcomm_new(NULL)) == NULL) {
		LWIP_DEBUGF(BT_SPP_DEBUG, ("bt_spp_init: Could not alloc RFCOMM PCB for channel 0\n"));
		return ERR_MEM;
	}
	rfcomm_listen(rfcommpcb, 0, rfcomm_accept);

	LWIP_DEBUGF(RFCOMM_DEBUG, ("bt_spp_init: Allocate RFCOMM PCB for CN 1\n"));
	if((rfcommpcb = rfcomm_new(NULL)) == NULL) {
		LWIP_DEBUGF(BT_SPP_DEBUG, ("lap_init: Could not alloc RFCOMM PCB for channel 1\n"));
		return ERR_MEM;
	}
	rfcomm_listen(rfcommpcb, 1, rfcomm_accept);

	if((record = sdp_record_new((u8_t *)ramona_service_record, sizeof(ramona_service_record))) == NULL) {
		LWIP_DEBUGF(BT_SPP_DEBUG, ("bt_spp_init: Could not alloc SDP record\n"));
		return ERR_MEM;
	} else {
		sdp_register_service(record);
	}

	LWIP_DEBUGF(BT_SPP_DEBUG, ("SPP initialized\n"));
	return ERR_OK;
}

err_t acl_wpl_complete(void *arg, struct bd_addr *bdaddr)
{
	hci_sniff_mode(bdaddr, 200, 100, 10, 10);
	return ERR_OK;
}

err_t acl_conn_complete(void *arg, struct bd_addr *bdaddr)
{
	hci_wlp_complete(acl_wpl_complete);
	hci_write_link_policy_settings(bdaddr, 0x000F);
	return ERR_OK;
}

err_t read_bdaddr_complete(void *arg, struct bd_addr *bdaddr)
{
	memcpy(&(bt_spp_state.bdaddr), bdaddr, 6);
	LWIP_DEBUGF(BT_SPP_DEBUG, ("read_bdaddr_complete: %02x:%02x:%02x:%02x:%02x:%02x\n",
				bdaddr->addr[5], bdaddr->addr[4], bdaddr->addr[3],
				bdaddr->addr[2], bdaddr->addr[1], bdaddr->addr[0]));

    /*{
        unsigned char buf[32];
        unsigned char *addr = bdaddr->addr;
        sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", addr[5], addr[4], addr[3],
                addr[2], addr[1], addr[0]);
        nolle_transmit(0x06, buf, strlen(buf));
    }*/

    return ERR_OK;
}

err_t command_complete(void *arg, struct hci_pcb *pcb, u8_t ogf, u8_t ocf, u8_t result)
{
    //u8_t cod_spp[] = {0x08,0x04,0x24}>; // Render/Audio, handsfree
	u8_t cod_spp[] = {0x00,0x00,0x04}; // Render, misc.
	u8_t devname[32];
	u8_t n1, n2, n3;
	u8_t flag = HCI_SET_EV_FILTER_AUTOACC_NOROLESW; //HCI_SET_EV_FILTER_AUTOACC_ROLESW;

	switch(ogf) {
		case HCI_INFO_PARAM:
			switch(ocf) {
				case HCI_READ_BUFFER_SIZE:
					if(result == HCI_SUCCESS) {
						LWIP_DEBUGF(BT_SPP_DEBUG, ("successful HCI_READ_BUFFER_SIZE.\n"));
						hci_read_bd_addr(read_bdaddr_complete);
					} else {
						LWIP_DEBUGF(BT_SPP_DEBUG, ("Unsuccessful HCI_READ_BUFFER_SIZE.\n"));
						return ERR_CONN;
					}
					break;
				case HCI_READ_BD_ADDR:
					if(result == HCI_SUCCESS) {
						LWIP_DEBUGF(BT_SPP_DEBUG, ("successful HCI_READ_BD_ADDR.\n"));
						/* Make discoverable */
						hci_set_event_filter(HCI_SET_EV_FILTER_CONNECTION,
								HCI_SET_EV_FILTER_ALLDEV, &flag);
					} else {
						LWIP_DEBUGF(BT_SPP_DEBUG, ("Unsuccessful HCI_READ_BD_ADDR.\n"));
						return ERR_CONN;
					}
					break;
				default:
					LWIP_DEBUGF(BT_SPP_DEBUG, ("Unknown HCI_INFO_PARAM command complete event\n"));
					break;
			}
			break;
		case HCI_HC_BB_OGF:
			switch(ocf) {
				case HCI_RESET:
					if(result == HCI_SUCCESS) {
						LWIP_DEBUGF(BT_SPP_DEBUG, ("successful HCI_RESET.\n")); 
						hci_read_buffer_size();
					} else {
						LWIP_DEBUGF(BT_SPP_DEBUG, ("Unsuccessful HCI_RESET.\n"));
						return ERR_CONN;
					}
					break;
				case HCI_WRITE_SCAN_ENABLE:
					if(result == HCI_SUCCESS) {
						LWIP_DEBUGF(BT_SPP_DEBUG, ("successful HCI_WRITE_SCAN_ENABLE.\n")); 
						//hci_cmd_complete(NULL); /* Initialization done, don't come back */
					} else {
						LWIP_DEBUGF(BT_SPP_DEBUG, ("Unsuccessful HCI_WRITE_SCAN_ENABLE.\n"));
						return ERR_CONN;
					}
					break;
				case HCI_SET_EVENT_FILTER:
					if(result == HCI_SUCCESS) {
						LWIP_DEBUGF(BT_SPP_DEBUG, ("successful HCI_SET_EVENT_FILTER.\n"));
							hci_write_cod(cod_spp);
							hci_write_scan_enable(HCI_SCAN_EN_INQUIRY | HCI_SCAN_EN_PAGE);
					} else {
						LWIP_DEBUGF(BT_SPP_DEBUG, ("Unsuccessful HCI_SET_EVENT_FILTER.\n"));
						return ERR_CONN;
					}
					break;
				case HCI_CHANGE_LOCAL_NAME:
					if(result == HCI_SUCCESS) {
						LWIP_DEBUGF(BT_SPP_DEBUG, ("Successful HCI_CHANGE_LOCAL_NAME.\n"));
						hci_write_page_timeout(0x4000); /* 10.24s */
					} else {
						LWIP_DEBUGF(BT_SPP_DEBUG, ("Unsuccessful HCI_CHANGE_LOCAL_NAME.\n"));
						return ERR_CONN;
					}
					break;
				case HCI_WRITE_COD:
					if(result == HCI_SUCCESS) {
						LWIP_DEBUGF(BT_SPP_DEBUG, ("Successful HCI_WRITE_COD.\n"));
                        sprintf(devname, "ramona-%02x%02x%02x",
                                bt_spp_state.bdaddr.addr[2],
                                bt_spp_state.bdaddr.addr[1],
                                bt_spp_state.bdaddr.addr[0]);
						hci_change_local_name(devname, strlen(devname)+1);
					} else {
						LWIP_DEBUGF(BT_SPP_DEBUG, ("Unsuccessful HCI_WRITE_COD.\n"));
						return ERR_CONN;
					}
					break;
				case HCI_WRITE_PAGE_TIMEOUT:
					if(result == HCI_SUCCESS) {
						LWIP_DEBUGF(BT_SPP_DEBUG, ("successful HCI_WRITE_PAGE_TIMEOUT.\n"));
						//hci_cmd_complete(NULL); /* Initialization done, don't come back */
						hci_connection_complete(acl_conn_complete);
						LWIP_DEBUGF(BT_SPP_DEBUG, ("Initialization done.\n"));
						//LWIP_DEBUGF(BT_SPP_DEBUG, ("Discover other Bluetooth devices.\n"));
						//hci_inquiry(0x009E8B33, 0x04, 0x01, inquiry_complete); //FAILED????
					} else {
						LWIP_DEBUGF(BT_SPP_DEBUG, ("Unsuccessful HCI_WRITE_PAGE_TIMEOUT.\n"));
						return ERR_CONN;
					}
					break;
				default:
					LWIP_DEBUGF(BT_SPP_DEBUG, ("Unknown HCI_HC_BB_OGF command complete event\n"));
					break;
			}
			break;
		default:
			LWIP_DEBUGF(BT_SPP_DEBUG, ("Unknown command complete event. OGF = 0x%x OCF = 0x%x\n", ogf, ocf));
			break;
	}
	return ERR_OK;
}

void lwbt_init(void)
{
    mem_init();
    memp_init();
    pbuf_init();

    lwbt_memp_init();
    transport_init();
    if (hci_init() != ERR_OK) {
		LWIP_DEBUGF(BT_SPP_DEBUG, ("lwbt_init: HCI Initialization failed\n"));
        OSE_UserError(0x99);
    }

    l2cap_init();
    sdp_init();
    rfcomm_init();

	hci_reset_all();
	l2cap_reset_all();
	sdp_reset_all();
	rfcomm_reset_all();

	hci_cmd_complete(command_complete);
	hci_pin_req(pin_req);
	bt_spp_state.btctrl = 0;
	bt_spp_state.p = NULL;
	hci_reset();

	if (bt_spp_init() != ERR_OK) {
		LWIP_DEBUGF(BT_SPP_DEBUG, ("lwbt_init: couldn't init role\n"));
        OSE_UserError(0x99);
	}
}

void lwbt_timer(void)
{
	l2cap_tmr();
	rfcomm_tmr();

    /*static int inquiry_timeout = 60;
    if ((inquiry_timeout) && (!(--inquiry_timeout))) {
        hci_write_scan_enable(HCI_SCAN_EN_PAGE);
    }*/
}