/*
 * Ericsson Baseband Controller
 * Definitions for OSE
 *
 * 2012-07-25 <fredrik@etf.nu>
 */

#ifndef __OSE_H__
#define __OSE_H__

#include <stddef.h>

typedef unsigned short SIGSELECT;
typedef unsigned short PROCESS;

union SIGNAL
{
    SIGSELECT   sig_no;
    unsigned char raw[0];
};
typedef union SIGNAL SIGNAL;

struct PROCINIT
{
    unsigned int type;  /* +0x00 */
    void *entry;        /* +0x04 */
    unsigned int prio;  /* +0x08 */
    void *ptr1;         /* +0x0c */
    void *pcb;          /* +0x10 */
    void *ptr2;         /* +0x14 */
    void *ptr3;         /* +0x18 */
    unsigned int pid;   /* +0x1c */
    char name[];
};
typedef struct PROCINIT PROCINIT;

extern SIGNAL *OSE_alloc(size_t size, SIGSELECT signal);
extern void OSE_free_buf(SIGNAL **s);
extern SIGNAL *OSE_receive(SIGSELECT *sigsel);
extern void OSE_send(SIGNAL **sig, PROCESS to);
extern unsigned int OSE_get_ticks(void);

#endif
