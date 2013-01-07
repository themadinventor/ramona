/*
 * Ericsson Baseband Controller
 * Entry points into the original firmware
 *
 * 2012-07-25 <fredrik@etf.nu>
 */

#ifndef __APP_H__
#define __APP_H__

#include "host/ose.h"

#define SIG_NVDS_ERASE  0x9C4
#define SIG_NVDS_WRITE  0x9C5
#define SIG_NVDS_READ   0x9C6
#define SIG_NVDS_RESULT 0x9C7
#define SIG_NVDS_DELETE 0x9C8

extern void RegisterTransport(int interface, int zero, void *send_event, void *send_acl, void *stub1, void *stub2, void *stub3);
extern void HCI_Trans_Event_Sent(int interface);
extern void HCI_Input_ACL(int interface, SIGNAL *s);
extern void HCI_Trans_ACL_Sent(int interface);

extern void timer_add(int seconds, SIGSELECT signal);

extern void I2C_Init();
extern int I2C_Read(unsigned char chip_id, unsigned char address, unsigned char *data);
extern int I2C_Write(unsigned char chip_id, unsigned char address, unsigned char data);

extern int NVDS_ReadFile(unsigned char tag, size_t len, void *ptr);
extern void NVDS_WriteFile(unsigned char tag, size_t len, void *ptr);
extern void NVDS_DeleteFile(unsigned char tag);

extern PROCESS proc_hci, proc_flash_eraser, proc_flash_handler;
extern char build_time[];
extern char build_comment[];

extern struct {
    void (*write)(void *addr, unsigned char data);  /* +00: write proc */
    void (*erase)(void *addr);                      /* +04: erase proc */
    unsigned char initialized;                      /* +08: set if successfully initialized */
    unsigned char current_page;                     /* +09: current page */
    void *tag_head;                                 /* +0c: head of tag list */
    void *blob_head;                                /* +10: head of blob */
    unsigned int free_space;                        /* +14: free space */
    void *pages[2];                                 /* +18: pages */
    unsigned int page_size;                         /* +20: page size */
} NVDS_Context;

extern struct {
    unsigned int code;
    unsigned int pid;
    unsigned int pcb;
    unsigned int sp;
    unsigned int domain;
} ose_crash_info;

#endif
