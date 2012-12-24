/*
 * Ericsson Baseband Controller
 * IRMA Backpack Entry Point
 *
 * 2012-07-25 <fredrik@etf.nu>
 */

.arm
.section .head

/* AIF Header */
magic:
    .int    0xe1a01007      /* +0: AIF Magic */
    nop                     /* +4: Blob hash */
entry:                      /* +8: ROM jumps here */
    nop
    bl      start

/* Secret header */
.thumb
    b       init_hook       /* +10: Init hook */
    b       uart2_rx_int    /* +12: UART2 Rx Int */
.int        backpackProc    /* +14: Process descriptor */
.int        procmem+2048    /* +18: Process PCB */

.balign 0x40
    .ascii  "Ramona FW"     /* +40: Comment */

/* Jump to main */
.arm
.balign 0x80                /* +80: Entry point */
start:
    ldr     r0, =0x01010004
    mov     pc, r0
