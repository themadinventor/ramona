/*
 * Ericsson Baseband Controller
 * UART Driver
 *
 * 2012-07-25 <fredrik@etf.nu>
 */

.global UARTInit
.thumb_func
UARTInit:
    mov     r0, #0x14
    ldr     r1, =0x00800d00
    strb    r0, [r1]
    ldr     r1, =0x00800d0c
    strb    r0, [r1]
    ldr     r1, =0x00800d14
    strb    r0, [r1]

    mov     r0, #0x2a
    ldr     r1, =0x00800d04
    strh    r0, [r1]

    mov     r0, #3
    ldr     r1, =0x0080000c
    strb    r0, [r1]
    ldr     r1, =0x0080010c
    strb    r0, [r1]
    ldr     r1, =0x0080020c
    strb    r0, [r1]

    ldr     r1, =0x00800014
    ldrb    r1, [r1]
    ldr     r1, =0x00800114
    ldrb    r1, [r1]
    ldr     r1, =0x00800214
    ldrb    r1, [r1]

    mov     r0, #0x01
    ldr     r1, =0x00800b04
    strb    r0, [r1]

    mov     r0, #0x80
    ldr     r1, =0x00800b08
    strb    r0, [r1]

    mov     r0, #0x01
    ldr     r1, =0x00800b14
    strb    r0, [r1]

    mov     r0, #0x80
    ldr     r1, =0x00800b18
    strb    r0, [r1]

    mov     r0, #0x01
    ldr     r1, =0x00800b24
    strb    r0, [r1]

    mov     r0, #0x80
    ldr     r1, =0x00800b28
    strb    r0, [r1]

    mov     r0, #0x01
    ldr     r1, =0x00800b34
    strb    r0, [r1]

    mov     r0, #0x80
    ldr     r1, =0x00800b38
    strb    r0, [r1]

    mov     r0, #0x87 //#0x01
    ldr     r1, =0x00800208
    strb    r0, [r1]

    mov     r0, #0x7 //#0x01
    ldr     r1, =0x00800204
    strb    r0, [r1]

    mov     pc, lr

.global UART2SetBaudRate
.thumb_func
UART2SetBaudRate:
    ldr     r1, =0x00800d0c
    strb    r0, [r1]
    mov     pc, lr

.global UART2WriteByte
.thumb_func
UART2WriteByte:
    ldr     r1, =0x00800b20
    strb    r0, [r1]
    mov     pc, lr

.global UART2ReadByte
.thumb_func
UART2ReadByte:
    ldr     r1, =0x00800b30
    ldrb    r0, [r1]
    mov     pc, lr

.global UART2GetTxFIFOSize
.thumb_func
UART2GetTxFIFOSize:
    ldr     r1, =0x00800b2c
    ldrb    r0, [r1]
    mov     pc, lr

.global UART2GetRxFIFOSize
.thumb_func
UART2GetRxFIFOSize:
    ldr     r1, =0x00800b3c
    ldrb    r0, [r1]
    mov     pc, lr

.global UART2GetChar
.thumb_func
UART2GetChar:
    push    {lr}
1:
    bl      UART2GetRxFIFOSize
    cmp     r0, #0
    beq     1b
    bl      UART2ReadByte
    pop     {pc}

.global UART2PutChar
.thumb_func
UART2PutChar:
    push    {r4,r5,lr}
    mov     r4, r0
    mov     r5, #128
1:
    bl      UART2GetTxFIFOSize
    sub     r0, r5, r0
    cmp     r0, #1
    blt     1b
    mov     r0, r4
    bl      UART2WriteByte
    pop     {r4,r5,pc}

.global UART2WriteString
.thumb_func
UART2WriteString:
    push    {r4,r5,r6,lr}
    mov     r4, r0
    mov     r5, #128
1:
    ldrb    r6, [r4]
    cmp     r6, #0
    beq     3f

2:
    bl      UART2GetTxFIFOSize
    sub     r0, r5, r0
    cmp     r0, #1
    blt     2b

    mov     r0, r6
    bl      UART2WriteByte

    add     r4, #1
    b       1b
3:
    pop     {r4,r5,r6,pc}

.global UART1SetBaudRate
.thumb_func
UART1SetBaudRate:
    ldr     r1, =0x00800d00
    strb    r0, [r1]
    mov     pc, lr

.global UART1WriteByte
.thumb_func
UART1WriteByte:
    ldr     r1, =0x00800b00
    strb    r0, [r1]
    mov     pc, lr

.global UART1ReadByte
.thumb_func
UART1ReadByte:
    ldr     r1, =0x00800b10
    ldrb    r0, [r1]
    mov     pc, lr

.global UART1GetTxFIFOSize
.thumb_func
UART1GetTxFIFOSize:
    //ldr     r1, =0x00800b1c
    ldr     r1, =0x00800b0c
    ldrb    r0, [r1]
    mov     pc, lr

.global UART1GetRxFIFOSize
.thumb_func
UART1GetRxFIFOSize:
    ldr     r1, =0x00800b1c
    ldrb    r0, [r1]
    mov     pc, lr

.global UART1GetChar
.thumb_func
UART1GetChar:
    push    {lr}
1:
    bl      UART1GetRxFIFOSize
    cmp     r0, #0
    beq     1b
    bl      UART1ReadByte
    pop     {pc}

.global UART1PutChar
.thumb_func
UART1PutChar:
    push    {r4,r5,lr}
    mov     r4, r0
    mov     r5, #128
1:
    bl      UART1GetTxFIFOSize
    sub     r0, r5, r0
    cmp     r0, #1
    blt     1b
    mov     r0, r4
    bl      UART1WriteByte
    pop     {r4,r5,pc}

.global UART1WriteString
.thumb_func
UART1WriteString:
    push    {r4,r5,r6,lr}
    mov     r4, r0
    mov     r5, #128
1:
    ldrb    r6, [r4]
    cmp     r6, #0
    beq     3f

2:
    bl      UART1GetTxFIFOSize
    sub     r0, r5, r0
    cmp     r0, #1
    blt     2b

    mov     r0, r6
    bl      UART1WriteByte

    add     r4, #1
    b       1b
3:
    pop     {r4,r5,r6,pc}

.global UART3SetBaudRate
.thumb_func
UART3SetBaudRate:
    ldr     r1, =0x00800d14
    strb    r0, [r1]
    mov     pc, lr

.global UART3WriteByte
.thumb_func
UART3WriteByte:
    ldr     r1, =0x00800200
    strb    r0, [r1]
    mov     pc, lr

.global UART3ReadByte
.thumb_func
UART3ReadByte:
    ldr     r1, =0x00800200
    ldrb    r0, [r1]
    mov     pc, lr

.global UART3GetRxFIFOSize
.thumb_func
UART3GetRxFIFOSize:
    ldr     r1, =0x00800214
    ldrb    r0, [r1]
    lsl     r0, r0, #31
    lsr     r0, r0, #31
    mov     pc, lr

.global UART3GetChar
.thumb_func
UART3GetChar:
    push    {lr}
1:
    bl      UART3GetRxFIFOSize
    cmp     r0, #0
    beq     1b
    bl      UART1ReadByte
    pop     {pc}

.global UART3PutChar
.thumb_func
UART3PutChar:
    push    {r4,r5,lr}
    mov     r4, r0
    /*ldr     r1, =0x00800208*/
    ldr     r1, =0x00800214
    ldr     r5, =0x20
1:
    ldrb    r0, [r1]
    and     r0, r5, r0
    beq     1b
    /*lsr     r0, r0, #2
    bcc     1b*/
    mov     r0, r4
    bl      UART3WriteByte
    pop     {r4,r5,pc}

.global UART3WriteString
.thumb_func
UART3WriteString:
    push    {r4,r5,lr}
    mov     r4, r0
1:
    ldrb    r5, [r4]
    cmp     r5, #0
    beq     3f

    mov     r0, r5
    bl      UART3PutChar

    add     r4, #1
    b       1b
3:
    pop     {r4,r5,pc}
