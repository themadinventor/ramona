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

#include "btstack.h"
#include "signals.h"

#define BT_SPP_DEBUG LWIP_DBG_OFF /* Controls debug messages */

struct bt_state {
	struct bd_addr bdaddr;
	struct pbuf *p;
	u8_t btctrl;
	u8_t cn;
} bt_spp_state;

const char *bt_get_bdaddr(void)
{
    return (const char *)&bt_spp_state.bdaddr;
}

err_t pin_req(void *arg, struct bd_addr *bdaddr)
{
	u8_t pin[] = "1234";
	LWIP_DEBUGF(BT_SPP_DEBUG, ("pin_req\n"));
	return hci_pin_code_request_reply(bdaddr, 4, pin);
}

err_t bt_rfcomm_recv(void *arg, struct rfcomm_pcb *pcb, struct pbuf *p, err_t err)
{
	struct pbuf *q = NULL;
	
	LWIP_DEBUGF(BT_SPP_DEBUG, ("spp_recv: p->len == %d p->tot_len == %d\n", p->len, p->tot_len));

	for (q = p; q != NULL; q = q->next) {
        rfcommproc_t proc = arg;
        proc(pcb, RFCOMM_RECEIVED, q->payload, q->len);
	}
	
    pbuf_free(p);
	
	return ERR_OK;
}

void bt_rfcomm_write(struct rfcomm_pcb *pcb, char *buf, int len)
{
	struct pbuf *q = pbuf_alloc(PBUF_RAW, len, PBUF_RAM);
    memcpy(q->payload, buf, len);

	if (rfcomm_cl(pcb)) {
        rfcomm_uih_credits(pcb, PBUF_POOL_SIZE - rfcomm_remote_credits(pcb), q);
	} else {
		rfcomm_uih(pcb, rfcomm_cn(pcb), q);
	}
	pbuf_free(q);
}

void bt_rfcomm_disconnect(struct rfcomm_pcb *pcb)
{
    rfcomm_disconnect(pcb);
}

err_t rfcomm_disconnected(void *arg, struct rfcomm_pcb *pcb, err_t err) 
{
	err_t ret = ERR_OK;

    rfcommproc_t proc = arg;
    proc(pcb, RFCOMM_DISCONNECTED, NULL, 0);

	LWIP_DEBUGF(BT_SPP_DEBUG, ("rfcomm_disconnected: CN = %d\n", rfcomm_cn(pcb)));
	if (rfcomm_cn(pcb) != 0) {
		; //ppp_lp_disconnected(pcb);
	}
	rfcomm_close(pcb);

	return ret;
}

err_t rfcomm_accept(void *arg, struct rfcomm_pcb *pcb, err_t err) 
{
	LWIP_DEBUGF(BT_SPP_DEBUG, ("rfcomm_accept: CN = %d\n", rfcomm_cn(pcb)));

	rfcomm_disc(pcb, rfcomm_disconnected);
	if (pcb->cn != 0) {
		//set recv callback
		rfcomm_recv(pcb, bt_rfcomm_recv);

        rfcommproc_t proc = arg;
        proc(pcb, RFCOMM_ACCEPTED, NULL, 0);
	}

	return ERR_OK;
}

static err_t bt_disconnect_ind(void *arg, struct l2cap_pcb *pcb, err_t err)
{
	err_t ret;

	LWIP_DEBUGF(BT_SPP_DEBUG, ("bt_disconnect_ind\n"));

	if (pcb->psm == SDP_PSM) { 
		sdp_lp_disconnected(pcb);
	} else if (pcb->psm == RFCOMM_PSM) {
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
	if (pcb->psm == SDP_PSM) {
		l2cap_recv(pcb, sdp_recv);
	} else if (pcb->psm == RFCOMM_PSM) {
		l2cap_recv(pcb, rfcomm_input);
	}
	return ERR_OK;  
}

struct rfcomm_pcb *bt_rfcomm_listen(u8_t cn, rfcommproc_t proc)
{
	struct rfcomm_pcb *pcb;

	LWIP_DEBUGF(RFCOMM_DEBUG, ("bt_spp_init: Allocate RFCOMM PCB\n"));
	if ((pcb = rfcomm_new(NULL)) == NULL) {
		LWIP_DEBUGF(BT_SPP_DEBUG, ("lap_init: Could not alloc RFCOMM PCB\n"));
		return NULL;
	}
    pcb->callback_arg = proc;
	rfcomm_listen(pcb, cn, rfcomm_accept);

    return pcb;
}

err_t bt_spp_init(void)
{
	struct l2cap_pcb *l2cappcb;
	struct rfcomm_pcb *rfcommpcb;

	if ((l2cappcb = l2cap_new()) == NULL) {
		LWIP_DEBUGF(BT_SPP_DEBUG, ("bt_spp_init: Could not alloc L2CAP PCB for SDP_PSM\n"));
		return ERR_MEM;
	}
	l2cap_connect_ind(l2cappcb, SDP_PSM, bt_connect_ind);

	if ((l2cappcb = l2cap_new()) == NULL) {
		LWIP_DEBUGF(BT_SPP_DEBUG, ("bt_spp_init: Could not alloc L2CAP PCB for RFCOMM_PSM\n"));
		return ERR_MEM;
	}
	l2cap_connect_ind(l2cappcb, RFCOMM_PSM, bt_connect_ind);

	LWIP_DEBUGF(RFCOMM_DEBUG, ("bt_spp_init: Allocate RFCOMM PCB for CN 0\n"));
	if ((rfcommpcb = rfcomm_new(NULL)) == NULL) {
		LWIP_DEBUGF(BT_SPP_DEBUG, ("bt_spp_init: Could not alloc RFCOMM PCB for channel 0\n"));
		return ERR_MEM;
	}
	rfcomm_listen(rfcommpcb, 0, rfcomm_accept);

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

    return ERR_OK;
}

err_t command_complete(void *arg, struct hci_pcb *pcb, u8_t ogf, u8_t ocf, u8_t result)
{
    //u8_t cod_spp[] = {0x08,0x04,0x24}>; // Render/Audio, handsfree
	u8_t cod_spp[] = {0x00,0x05,0x00}; // Peripheral
	u8_t devname[32];
	u8_t n1, n2, n3;
	u8_t flag = HCI_SET_EV_FILTER_AUTOACC_NOROLESW; //HCI_SET_EV_FILTER_AUTOACC_ROLESW;

	switch (ogf) {
		case HCI_INFO_PARAM:
			switch (ocf) {
				case HCI_READ_BUFFER_SIZE:
					if (result == HCI_SUCCESS) {
						LWIP_DEBUGF(BT_SPP_DEBUG, ("successful HCI_READ_BUFFER_SIZE.\n"));
						hci_read_bd_addr(read_bdaddr_complete);
					} else {
						LWIP_DEBUGF(BT_SPP_DEBUG, ("Unsuccessful HCI_READ_BUFFER_SIZE.\n"));
						return ERR_CONN;
					}
					break;
				case HCI_READ_BD_ADDR:
					if (result == HCI_SUCCESS) {
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
			switch (ocf) {
				case HCI_RESET:
					if (result == HCI_SUCCESS) {
						LWIP_DEBUGF(BT_SPP_DEBUG, ("successful HCI_RESET.\n")); 
						hci_read_buffer_size();
					} else {
						LWIP_DEBUGF(BT_SPP_DEBUG, ("Unsuccessful HCI_RESET.\n"));
						return ERR_CONN;
					}
					break;
				case HCI_WRITE_SCAN_ENABLE:
					if (result == HCI_SUCCESS) {
						LWIP_DEBUGF(BT_SPP_DEBUG, ("successful HCI_WRITE_SCAN_ENABLE.\n")); 
						//hci_cmd_complete(NULL); /* Initialization done, don't come back */
					} else {
						LWIP_DEBUGF(BT_SPP_DEBUG, ("Unsuccessful HCI_WRITE_SCAN_ENABLE.\n"));
						return ERR_CONN;
					}
					break;
				case HCI_SET_EVENT_FILTER:
					if (result == HCI_SUCCESS) {
						LWIP_DEBUGF(BT_SPP_DEBUG, ("successful HCI_SET_EVENT_FILTER.\n"));
							hci_write_cod(cod_spp);
							hci_write_scan_enable(HCI_SCAN_EN_INQUIRY | HCI_SCAN_EN_PAGE);
					} else {
						LWIP_DEBUGF(BT_SPP_DEBUG, ("Unsuccessful HCI_SET_EVENT_FILTER.\n"));
						return ERR_CONN;
					}
					break;
				case HCI_CHANGE_LOCAL_NAME:
					if (result == HCI_SUCCESS) {
						LWIP_DEBUGF(BT_SPP_DEBUG, ("Successful HCI_CHANGE_LOCAL_NAME.\n"));
						hci_write_page_timeout(0x2000); /* 5.12s */
					} else {
						LWIP_DEBUGF(BT_SPP_DEBUG, ("Unsuccessful HCI_CHANGE_LOCAL_NAME.\n"));
						return ERR_CONN;
					}
					break;
				case HCI_WRITE_COD:
					if (result == HCI_SUCCESS) {
						LWIP_DEBUGF(BT_SPP_DEBUG, ("Successful HCI_WRITE_COD.\n"));
                        sprintf(devname, "irma-%02x%02x%02x",
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
					if (result == HCI_SUCCESS) {
						LWIP_DEBUGF(BT_SPP_DEBUG, ("successful HCI_WRITE_PAGE_TIMEOUT.\n"));
						hci_cmd_complete(NULL); /* Initialization done, don't come back */
						hci_connection_complete(acl_conn_complete);
						LWIP_DEBUGF(BT_SPP_DEBUG, ("Initialization done.\n"));
						//LWIP_DEBUGF(BT_SPP_DEBUG, ("Discover other Bluetooth devices.\n"));
						//hci_inquiry(0x009E8B33, 0x04, 0x01, inquiry_complete); //FAILED????
                        btstack_init_complete();
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
        UserError(0x99);
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
        UserError(0x99);
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
