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

void teardown(void)
{
    printf("connect: teardown\n");
}

err_t inquiry_complete(void *arg, struct hci_pcb *pcb, struct hci_inq_res *ires, u16_t result)
{
    if (result == HCI_SUCCESS) {
        printf("connect: inquiry successful\n");

        if (ires != NULL) {
            printf("connect: result\n");
            printf("  psrm %d\n  psm %d\n  co %d\n",
                    ires->psrm, ires->psm, ires->co);
            printf("  bdaddr %02x:%02x:%02x:%02x:%02x:%02x\n",
                    ires->bdaddr.addr[5], ires->bdaddr.addr[4], ires->bdaddr.addr[3],
                    ires->bdaddr.addr[2], ires->bdaddr.addr[1], ires->bdaddr.addr[0]);
        } else {
            //hci_inquiry(0x009E8B33, 0x04, 0x01, inquiry_complete);
            printf("No result.\n");
        }
    } else {
        printf("connect: inquiry failed\n");
        hci_inquiry(0x009E8B33, 0x04, 0x01, inquiry_complete);
    }
}

int start(int p)
{
    printf("connect: start\n");

    plugin_teardown(teardown);

    hci_inquiry(0x009E8B33, 0x04, 0x01, inquiry_complete);

    printf("connect: initialization complete\n");

    return 0;
}

