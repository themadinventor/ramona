/*
 * Ericsson Baseband Controller
 * Definitions for OSE
 *
 * 2012-07-25 <fredrik@etf.nu>
 */

#ifndef __OSE_H__
#define __OSE_H__

typedef unsigned short SIGSELECT;
typedef unsigned short PROCESS;
typedef unsigned short OSPRIORITY;
typedef unsigned short OSBUFSIZE;
typedef unsigned short OSFSEMVAL;
typedef unsigned short OSSEMVAL;
typedef unsigned int OSTIME;

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
    void *readyq;       /* +0x0c */
    void *pcb;          /* +0x10 */
    void *stack_limit;  /* +0x14 */
    void *stack_base;   /* +0x18 */
    unsigned int pid;   /* +0x1c */
    char name[];
};
typedef struct PROCINIT PROCINIT;

typedef void SEMAPHORE;

extern SIGNAL	*alloc(OSBUFSIZE size, SIGSELECT signal);
extern void	UserError(unsigned int errno);
extern void	free_buf(SIGNAL **s);
extern SIGNAL	*receive(SIGSELECT *sigsel);
extern void	delay(OSTIME ticks);
extern void	start(PROCESS pid);
extern void	stop(PROCESS pid);
extern void	set_fsem(OSFSEMVAL val, PROCESS pid);
extern void	wait_fsem(void);
extern void	signal_fsem(PROCESS pid);
extern OSFSEMVAL get_fsem(PROCESS pid);
extern SEMAPHORE *create_sem(OSSEMVAL initial);
extern void	wait_sem(SEMAPHORE *sem);
extern void	signal_sem(SEMAPHORE *sem);
extern OSSEMVAL get_sem(SEMAPHORE *sem);
extern void	kill_sem(SEMAPHORE **sem);
extern PROCESS	addressee(SIGNAL **sig);
extern PROCESS	current_process(void);
extern OSTIME	get_ticks(void);
extern void	tick(void);
extern OSPRIORITY get_pri(PROCESS pid);
extern void	set_pri(PROCESS pid, OSPRIORITY pri);
extern void	send(SIGNAL **sig, PROCESS to);
extern PROCESS	sender(SIGNAL **sig);
extern OSBUFSIZE sigsize(SIGNAL **sig);

extern void readyq_1, readyq_2, readyq_3, readyq_4, readyq_31;

#endif
