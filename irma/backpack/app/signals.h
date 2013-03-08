#ifndef __SIGNALS_H__
#define __SIGNALS_H__

#define PID_BACKPACK        0x15

#define SIG_TIMER_1S        0xc000
#define SIG_TRANSPORT_EVENT 0xc001
#define SIG_TRANSPORT_DATA  0xc002
#define SIG_UART_RX         0xc003
#define SIG_FLASH_ERASE     0xc004
#define SIG_FLASH_WRITE     0xc005
#define SIG_FLASH_COMPLETE  0xc006
#define SIG_TIMER_LEDBLINK  0xc007

struct sig_flash_erase {
    SIGSELECT sig_no;
    void *addr;
};

struct sig_flash_write {
    SIGSELECT sig_no;
    void *dst, *src;
    size_t len;
};

/*struct sig_bt_accepted {
    SIGSELECT sig_no;
    struct rfcomm_pcb *pcb;
};

struct sig_bt_disconnected {
    SIGSELECT sig_no;
    struct rfcomm_pcb *pcb;
};

struct sig_bt_received {
    SIGSELECT sig_no;
    struct rfcomm_pcb *pcb;
    size_t len;
    char data[0];
};

struct sig_bt_send {
    SIGSELECT sig_no;
    struct rfcomm_pcb *pcb;
    size_t len;
    char data[0];
};*/

#endif
