#ifndef __BTSTACK_H
#define __BTSTACK_H

void lwbt_init(void);
void lwbt_timer(void);

const char *bt_get_bdaddr(void);

typedef void (*rfcommproc_t)(struct rfcomm_pcb *, int, void *, size_t);

struct rfcomm_pcb *bt_rfcomm_listen(u8_t cn, rfcommproc_t proc);
void bt_rfcomm_write(struct rfcomm_pcb *pcb, char *buf, int len);
void bt_rfcomm_disconnect(struct rfcomm_pcb *pcb);

enum {
    RFCOMM_ACCEPTED,
    RFCOMM_RECEIVED,
    RFCOMM_DISCONNECTED
};


#endif
