/*
 * Ericsson Baseband Controller
 * Utility functions
 *
 * 2012-07-25 <fredrik@etf.nu>
 */

.thumb

NibbleToHex:
    lsl     r0, r0, #28
    lsr     r0, r0, #28
    cmp     r0, #9
    bgt     .L1
    add     r0, #0x30
    mov     pc, lr
.L1:
    add     r0, #0x37
    mov     pc, lr

.global WriteHexByte
.thumb_func
WriteHexByte:
    push    {r6, r7, lr}
    mov     r7, r0
    mov     r6, #4
    b       .L2

.global WriteHex
.thumb_func
WriteHex:
    push    {r6, r7, lr}
    mov     r7, r0
    mov     r6, #28

.L2:
    mov     r0, r7
    lsr     r0, r6
    bl      NibbleToHex
    bl      UART2PutChar
    cmp     r6, #0
    beq     .L3
    sub     r6, #4
    b       .L2

.L3:
    pop     {r6, r7, pc}
