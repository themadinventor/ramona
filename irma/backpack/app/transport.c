/*
 * Ericsson Bluetooth Baseband Controller
 * Transport wrapper
 *
 * (c) 2012 <fredrik@z80.se>
 */

#include "host/ose.h"
#include "host/app.h"

#include "uart.h"
#include "utils.h"
#include "transport.h"
#include "signals.h"

#include "arch/lwbtopts.h"
#include "lwbt/phybusif.h"
#include "lwbt/hci.h"
#include "lwip/debug.h"
#include "lwip/mem.h"

#include <string.h>

static int transport_registered;

void transport_send_cmd(SIGNAL *s);

static void transport_if_send_hci_event(int interface, void *packet, int length);
static void transport_if_send_hci_data(int interface, void *packet, int length);
static void transport_if_stub1(void);
static void transport_if_stub2(void);
static void transport_if_stub3(void);

#define TRANSPORT_IF    2

void transport_init(void)
{
    if (!transport_registered) {
        LWIP_DEBUGF(PHYBUSIF_DEBUG, ("Trying to register transport..."));

        RegisterTransport(TRANSPORT_IF, 0, transport_if_send_hci_event, transport_if_send_hci_data,
                transport_if_stub1, transport_if_stub2, transport_if_stub3);

        LWIP_DEBUGF(PHYBUSIF_DEBUG, ("done\r\n"));

        transport_registered = 0xcafebabe;
    } else {
        LWIP_DEBUGF(PHYBUSIF_DEBUG, ("Transport already installed = %08x\n", transport_registered));
    }
}

extern unsigned short bppid;

static void transport_if_send_hci_event(int interface, void *packet, int length)
{
    LWIP_DEBUGF(PHYBUSIF_DEBUG, ("HCI Event: interface=%d, packet=%08x, length=%d\n", interface, packet, length));

    if (length > 255) {
        //OSE_UserError(0xef);
        printf("HCI Event: Packet bigger than 255 bytes, dropping! (len=%d)\n", length);
        return;
    }

    SIGNAL *s = alloc(2+255, SIG_TRANSPORT_EVENT);
    if (!s) {
        LWIP_DEBUGF(PHYBUSIF_DEBUG, ("HCI Event: Unable to allocate signal, dropping.\n"));
        return;
    }

    memcpy(&s->raw[2], packet, length);
    send(&s, PID_BACKPACK);
    HCI_Trans_Event_Sent(TRANSPORT_IF);
}

static void transport_if_send_hci_data(int interface, void *packet, int length)
{
    unsigned int lr;
    asm ("mov %0, lr":"=r" (lr));

    LWIP_DEBUGF(PHYBUSIF_DEBUG, ("HCI Data: interface=%d, packet=%08x, length=%d, lr=%08x\n", interface, packet, length, lr));

    if (length > 804) {
        //OSE_UserError(0xee);
        printf("HCI Data: Packet bigger than 804 bytes, dropping! (len=%d)\n", length);
        return;
    }

    SIGNAL *s = alloc(806, SIG_TRANSPORT_DATA);
    if (!s) {
        LWIP_DEBUGF(PHYBUSIF_DEBUG, ("HCI Data: Unable to allocate signal, dropping.\n"));
        return;
    }

    memcpy(&s->raw[2], packet, length);
    send(&s, PID_BACKPACK);
    HCI_Trans_ACL_Sent(TRANSPORT_IF);
}

static void transport_if_stub1(void)
{
    //LWIP_DEBUGF(PHYBUSIF_DEBUG, ("%s\n", __FUNCTION__));
}

static void transport_if_stub2(void)
{
    //LWIP_DEBUGF(PHYBUSIF_DEBUG, ("%s\n", __FUNCTION__));
}

static void transport_if_stub3(void)
{
    //LWIP_DEBUGF(PHYBUSIF_DEBUG, ("%s\n", __FUNCTION__));
}

int transport_input(SIGNAL *s)
{
    int type = (s->sig_no == SIG_TRANSPORT_EVENT) ? 0 : 1;

    void *buf = &s->raw[2];
    size_t length = (type) ? (s->raw[4] | (s->raw[5] << 8))+4 : s->raw[3]+2;

    struct pbuf *p, *q;
	if ((p = pbuf_alloc(PBUF_RAW, PBUF_POOL_BUFSIZE, PBUF_POOL)) == NULL) {
		LWIP_DEBUGF(PHYBUSIF_DEBUG, ("transport_input: Could not allocate memory for pbuf\n"));
		return ERR_MEM;
	}
	q = p;

    LWIP_DEBUGF(PHYBUSIF_DEBUG, ("transport_input: type=%02x, buf=%08x, length=%04x\n", type, buf, length));

    char *ptr = buf;
    int bufidx = 0;
    while (length--) {
        //LWIP_DEBUGF(PHYBUSIF_DEBUG, ("%02x ", *ptr));

        ((u8_t *)q->payload)[bufidx++] = *ptr++;

        if (bufidx == q->len) {
			bufidx = 0;
			if((q = pbuf_alloc(PBUF_RAW, PBUF_POOL_BUFSIZE, PBUF_POOL)) == NULL) {
				LWIP_DEBUGF(PHYBUSIF_DEBUG, ("transport_input: Could not allocate memory for event parameter pbuf\n"));
				return ERR_MEM; /* Could not allocate memory for pbuf */
			}

			pbuf_chain(p, q);
			pbuf_free(q);
        }
    }
    LWIP_DEBUGF(PHYBUSIF_DEBUG, ("\n"));
    
    if (type) {
        pbuf_header(p, -HCI_ACL_HDR_LEN);
		hci_acl_input(p); /* Handle incoming ACL data */
    } else {
        pbuf_header(p, -HCI_EVENT_HDR_LEN);
        hci_event_input(p); /* Handle incoming event */
    }

    if (!type) {
    //printf("*** transport: processing done, freeing (type=%02x)\n", type);
	pbuf_free(p);
    //printf("*** transport: freed (type=%02x)\n", type);
    }

	return ERR_OK;
}

void phybusif_output(struct pbuf *p, u16_t len) 
{
	static struct pbuf *q;
	static int i;
	static unsigned char *ptr;
	unsigned char c, type, idx = 0;
	unsigned int inited = 0;
    SIGNAL *s;

    LWIP_DEBUGF(PHYBUSIF_DEBUG, ("phybusif_output: len = %d\n", len));

	for(q = p; q != NULL; q = q->next) {
		ptr = q->payload;
		for(i = 0; i < q->len && len; i++) {
			c = *ptr++;

            if (inited) {
			    //LWIP_DEBUGF(PHYBUSIF_DEBUG, ("%02x ", c));

                s->raw[2+idx++] = c;
    
                /*if (idx > 255) {
                    OSE_UserError(0xdd);
                }*/
            } else {
                type = c;
                //LWIP_DEBUGF(PHYBUSIF_DEBUG, ("[%02x] ", type));

                if (type == 0x01) {
                    s = alloc(5+255, 0x9d); // HCI_Cmd
                } else if (type == 0x02) {
                    s = alloc(806, 0x94); // HCI_ACL
                } else {
                    LWIP_DEBUGF(PHYBUSIF_DEBUG, ("phybusif_output: unknown hci packet type %02x!\n", type));
                    for (;;) ;
                }

                inited = 1;
            }

			--len;
		}
	}

	//LWIP_DEBUGF(PHYBUSIF_DEBUG, ("\n"));

    if (type == 0x02) {
        HCI_Input_ACL(TRANSPORT_IF, s);
    } else {
        send(&s, proc_hci);
    }
}
