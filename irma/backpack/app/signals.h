#ifndef __SIGNALS_H__
#define __SIGNALS_H__

#define PID_BACKPACK        0x15

#define SIG_TIMER           0xc000
#define SIG_TRANSPORT_EVENT 0xc001
#define SIG_TRANSPORT_DATA  0xc002
#define SIG_UART_RX         0xc003
#define SIG_FLASH_ERASE     0xc004
#define SIG_FLASH_WRITE     0xc005
#define SIG_FLASH_COMPLETE  0xc006

struct sig_erase {
    SIGSELECT sig_no;
    void *addr;
};

struct sig_write {
    SIGSELECT sig_no;
    void *dst, *src;
    size_t len;
};

#endif
