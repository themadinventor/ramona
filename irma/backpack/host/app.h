/*
 * Ericsson Baseband Controller
 * Entry points into the original firmware
 *
 * 2012-07-25 <fredrik@etf.nu>
 */

#ifndef __APP_H__
#define __APP_H__

#include "host/ose.h"

extern void RegisterTransport(int interface, int zero, void *send_event, void *send_acl, void *stub1, void *stub2, void *stub3);
extern void HCI_Trans_Event_Sent(int interface);
extern void HCI_Input_ACL(int interface, SIGNAL *s);
extern void HCI_Trans_ACL_Sent(int interface);

extern void timer_add(int seconds, SIGSELECT signal);

extern void I2C_Init();
extern int I2C_Read(unsigned char chip_id, unsigned char address, unsigned char *data);
extern int I2C_Write(unsigned char chip_id, unsigned char address, unsigned char data);

extern PROCESS proc_hci;
extern char build_time[];
extern char build_comment[];

#endif
