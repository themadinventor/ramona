#include "host/ose.h"
#include "host/app.h"

#include "uart.h"
#include "utils.h"
#include "signals.h"
#include "transport.h"
#include "plugin.h"

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

// Ramona Control: ea5cf8a3-833d-4d61-bfec-9cf6f4230ac4

static const u8_t ramona_service_record[] =
{
		SDP_DES_SIZE8, 0x8, 
			SDP_UINT16, 0x0, 0x0, /* Service record handle attribute */
				SDP_UINT32, 0x00, 0x00, 0x00, 0x00, /*dummy vals, filled in on xmit*/ 
		SDP_DES_SIZE8, 0x16, 
			SDP_UINT16, 0x0, 0x1, /* Service class ID list attribute */
			SDP_DES_SIZE8, 17,
				SDP_UUID128, 0xea, 0x5c, 0xf8, 0xa3,
                    0x83, 0x3d,
                    0x4d, 0x61,
                    0xbf, 0xec,
                    0x9c, 0xf6, 0xf4, 0x23, 0x0a, 0xc4,
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

enum {
    MON_CONNECTED = 1,      // payload = firmware info string, len = variable
    MON_DISCONNECT,
    MON_ERROR,
    MON_READMEM,            // payload = struct, len = variable
    MON_WRITEMEM,           // payload = struct, len = variable
    MON_WRITEFLASH,
    MON_ERASEFLASH,
    MON_REBOOT,
    MON_READREG,
    MON_WRITEREG,
    MON_CALL,
    MON_PLUGIN
};

struct monitor_packet {
    uint16_t type, len;
    union {
        uint8_t raw[0];
        struct {
            uint32_t addr;
            uint32_t len;
            uint8_t data[0];
        } memop;
        struct {
            uint32_t addr;
            uint32_t len;
            uint8_t data[0];
        } flashop;
        struct {
            uint32_t addr;
            uint32_t data;
        } regop;
        struct {
            uint32_t addr;
            uint32_t regs[4];
        } callop;
        struct {
            uint32_t state;
        } pluginop;
    };
} __attribute__((__packed__));

#define MON_PACK_SIZE 4

static struct monitor_packet inpkt;
uint8_t instate;

int mon_disconnect(struct rfcomm_pcb *pcb, void *ptr, size_t *len)
{
    // dangerous?
    bt_rfcomm_disconnect(pcb);

    return 0;
}

int mon_readmem(struct rfcomm_pcb *pcb, void *ptr, size_t *len)
{
    /*uint8_t outbuf[266];
    struct monitor_packet *outpkt = outbuf;
    memcpy(outpkt, &inpkt, 10);*/

    /*int idx;
    uint8_t *p = (uint8_t *)inpkt.memop.addr;
    for (idx=0; idx<inpkt.memop.len; idx++) {
        outpkt->memop.data[idx] = *p++;
    }*/
    //memcpy(outpkt->memop.data, (uint8_t *)inpkt.memop.addr, inpkt.memop.len);

    //outpkt->len = 8 + inpkt.memop.len;
    //bt_rfcomm_write(pcb, outpkt, MON_PACK_SIZE + outpkt->len);
    
    inpkt.len = 8 + inpkt.memop.len;
    //inpkt.memop.addr = 0;
    //inpkt.memop.len = 0;
    bt_rfcomm_write(pcb, &inpkt, MON_PACK_SIZE + 8);

    bt_rfcomm_write(pcb, (void *) inpkt.memop.addr, inpkt.memop.len);

    return 0;
}

int mon_writemem(struct rfcomm_pcb *pcb, void *ptr, size_t *len)
{
    size_t to_write = *len;
    if (to_write > inpkt.len - 8) {
        to_write = inpkt.len - 8;
    }

    memcpy((uint8_t *)inpkt.memop.addr, ptr, to_write);
    inpkt.memop.addr += to_write;

    *len -= to_write;
    inpkt.len -= to_write;

    if (inpkt.len == 8) {
        inpkt.len = 8;
        inpkt.memop.addr = 0;
        inpkt.memop.len = 0;
        bt_rfcomm_write(pcb, &inpkt, MON_PACK_SIZE + 8);
        return 0;
    } else {
        return 1;
    }
}

int mon_writeflash(struct rfcomm_pcb *pcb, void *ptr, size_t *len)
{
    size_t to_write = *len;
    if (to_write > inpkt.len - 8) {
        to_write = inpkt.len - 8;
    }

    //printf("writeflash: to_write=%d, len = %d\n", to_write, inpkt.len);

    Flash_Write(inpkt.flashop.addr, ptr, to_write);
    inpkt.flashop.addr += to_write;

    *len -= to_write;
    inpkt.len -= to_write;

    if (inpkt.len == 8) {
        inpkt.len = 8;
        inpkt.memop.addr = 0;
        inpkt.memop.len = 0;
        bt_rfcomm_write(pcb, &inpkt, MON_PACK_SIZE + 8);
        return 0;
    } else {
        return 1;
    }
}

int mon_eraseflash(struct rfcomm_pcb *pcb, void *ptr, size_t *len)
{
    size_t to_write = *len;
    if (to_write > inpkt.len - 8) {
        to_write = inpkt.len - 8;
    }

    uint32_t addr;
    for (addr=inpkt.flashop.addr; addr<inpkt.flashop.addr+inpkt.flashop.len; addr += 0x10000) {
        Flash_ErasePage(addr);
    }

    inpkt.len = 8;
    inpkt.memop.addr = 0;
    inpkt.memop.len = 0;
    bt_rfcomm_write(pcb, &inpkt, MON_PACK_SIZE + 8);
    return 0;
}

int mon_reboot(struct rfcomm_pcb *pcb, void *ptr, size_t *len)
{
    inpkt.len = 0;
    bt_rfcomm_write(pcb, &inpkt, MON_PACK_SIZE);
    bt_rfcomm_disconnect(pcb);

#if 1
    *((unsigned int *)0x00800c0c) = 0xc0;
    *((unsigned int *)0x00800c0c) = 0x1c;
#else
    *((unsigned int *)0x00800c10) = 0x00;
    *((unsigned int *)0x00800c0c) = 0xc0;
    *((unsigned int *)0x00800c0c) = 0x18;
#endif

    return 0;
}

int mon_readreg(struct rfcomm_pcb *pcb, void *ptr, size_t *len)
{
    inpkt.len = 8;
    inpkt.regop.data = *((uint32_t *)inpkt.regop.addr);
    bt_rfcomm_write(pcb, &inpkt, MON_PACK_SIZE + 8);

    return 0;
}

int mon_writereg(struct rfcomm_pcb *pcb, void *ptr, size_t *len)
{
    *((uint32_t *)inpkt.regop.addr) = inpkt.regop.data;
    inpkt.len = 0;
    bt_rfcomm_write(pcb, &inpkt, MON_PACK_SIZE);

    return 0;
}

int mon_call(struct rfcomm_pcb *pcb, void *ptr, size_t *len)
{
    uint32_t (*proc)(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3);

    inpkt.len = 8;

    proc = (void *) inpkt.callop.addr;
    inpkt.callop.regs[0] = proc(inpkt.callop.regs[0], inpkt.callop.regs[1], inpkt.callop.regs[2], inpkt.callop.regs[3]);

    bt_rfcomm_write(pcb, &inpkt, MON_PACK_SIZE+8);

    return 0;
}

int mon_plugin(struct rfcomm_pcb *pcb, void *ptr, size_t *len)
{
    inpkt.len = 4;

    //printf("mon_plugin: %08x\n", inpkt.pluginop.state);

    if (inpkt.pluginop.state == 1) {
        plugin_enable();
    } else if (inpkt.pluginop.state == 2) {
        plugin_disable();
    }

    inpkt.pluginop.state = plugin_enabled();
    bt_rfcomm_write(pcb, &inpkt, MON_PACK_SIZE+4);

    return 0;
}

const struct {
    int (*proc)(struct rfcomm_pcb *pcb, void *ptr, size_t *len);
    uint8_t hsize;
} monprocs[] = {
    {NULL, 0},                  // dummy.
    {NULL, 0},                  // MON_CONNECTED
    {mon_disconnect, 0},
    {NULL, 0},                  // MON_ERROR
    {mon_readmem, 8},
    {mon_writemem, 8},
    {mon_writeflash, 8},
    {mon_eraseflash, 8},
    {mon_reboot, 0},
    {mon_readreg, 4},
    {mon_writereg, 8},
    {mon_call, 20},
    {mon_plugin, 4}
};

void monitor_proc(struct rfcomm_pcb *pcb, int event, void *ptr, size_t len)
{
    if (event == RFCOMM_ACCEPTED) {
        /*uint8_t outbuf[66];
        struct monitor_packet *outpkt = outbuf;

        outpkt->type = MON_CONNECTED;
        sprintf(outpkt->raw, "Ramona Monitor. IRMA Build: %s, %s", bp_build_time, bp_build_comment);
        outpkt->len = strlen(outpkt->raw);
        bt_rfcomm_write(pcb, outpkt, MON_PACK_SIZE + outpkt->len);*/

        inpkt.type = MON_CONNECTED;
        inpkt.len = strlen(bp_build_time) + 1 + strlen(bp_build_comment) + 1;
        bt_rfcomm_write(pcb, &inpkt, MON_PACK_SIZE);
        bt_rfcomm_write(pcb, bp_build_time, strlen(bp_build_time) + 1);
        bt_rfcomm_write(pcb, bp_build_comment, strlen(bp_build_comment) + 1);

        instate = 0;

    } else if (event == RFCOMM_RECEIVED) {
        uint8_t *p = ptr;
        static uint16_t hidx;

        while (len > 0) {
            //printf("monitor_proc: instate=%d, len=%d\n", instate, len);
            if (instate == 0) {
                inpkt.type = *p++; len--;
                instate++;
            } else if (instate == 1) {
                //inpkt.type |= (*p++) << 8; len--;
                p++; len--;
                instate++;
            } else if (instate == 2) {
                inpkt.len = *p++; len--;
                instate++;
            } else if (instate == 3) {
                inpkt.len |= (*p++) << 8; len--;
                hidx = 0;
                if (monprocs[inpkt.type].hsize) {
                    instate++;
                } else {
                    instate = 5;
                }
            } else if (instate == 4) {
                inpkt.raw[hidx++] = *p++; len--;
                if (hidx == monprocs[inpkt.type].hsize) {
                    instate++;
                } else {
                    continue;
                }
            }
            if (instate == 5) {
                if (monprocs[inpkt.type].proc == NULL) {
                    instate = 0;
                    bt_rfcomm_disconnect(pcb);
                    return;
                }

                //printf("monitor_proc: invoking, type=%02x, len=%d\n", inpkt.type, len);
                ptr = p;
                if (!monprocs[inpkt.type].proc(pcb, ptr, &len)) {
                    instate = 0;
                }
            }
        }
    }
}

void monitor_init()
{
    bt_rfcomm_listen(1, monitor_proc);

	struct sdp_record *record;
	if ((record = sdp_record_new((u8_t *)ramona_service_record, sizeof(ramona_service_record))) != NULL) {
		sdp_register_service(record);
	}
}

