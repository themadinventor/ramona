.arm
    mov     r0, #2
    str     r0, [r1, #0x920]

    mov     r0, #7
    str     r0, [r1, #0x900]

    /* Send one byte */
    mov     r0, #0x40
    strb    r0, [r11, #0x00]
1:  ldrb    r0, [r11, #0x14]
    ands    r0, r0, #0x20
    beq     1b

    /* Flush uart */
2:  ldrb    r0, [r11, #0x14]
    mov     r3, #0x60
    and     r0, r3
    cmp     r0, #0x60
    bne     2b

    /* Change baud rate divisor */
    mov     r0, #0x00
    strb    r0, [r1, #0xd00]
    strb    r0, [r1, #0xd0c]
    strb    r0, [r1, #0xd14]

    /* Wait for byte */
3:  ldrb    r0, [r11, #0x14]
    ands    r0, r0, #0x01
    beq     3b
    ldrb    r0, [r11]

    ldr     r1, =0x00c003c0
    bx      r1
