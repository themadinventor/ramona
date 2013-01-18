.thumb
.global flash_identify
flash_identify:
    /* Unlock flash */
    /*ldr     r0, =0x0080314
    ldrb    r1, [r0]
    mov     r2, #0xef
    and     r1, r2
    strb    r1, [r0]*/

    /* Issue Read Identifier */
    ldr     r0, =0x01000000
    mov     r2, #0x90
    strb    r2, [r0]

    /* Read vendor */
    ldrh    r3, [r0]
    lsl     r3, #16

    /* Read product */
    ldrh    r2, [r0, #2]
    orr     r3, r2

    /* Issue Read Array */
    mov     r2, #0xff
    strb    r2, [r0]

    /* Lock flash */
    /*ldr     r0, =0x0080314
    ldrb    r1, [r0]
    mov     r2, #0x10
    orr     r1, r2
    strb    r1, [r0]*/

    mov     r0, r3
    bx      lr

.global flash_erase
flash_erase:
    /* Issue Erase Setup */
    mov     r1, #0x20
    strb    r1, [r0]

    /* Issue Erase Confirm */
    mov     r1, #0xd0
    strb    r1, [r0]

    /* Wait */
1:  ldrb    r1, [r0]
    mov     r2, #0x80
    and     r1, r2
    beq     1b

    /* Read status */
    ldrb    r1, [r0]
    mov     r2, #0x22
    and     r1, r2

    /* Issue Read Array */
    mov     r2, #0xff
    strb    r2, [r0]
    mov     r0, r1
    bx      lr

.global flash_write
flash_write:
    /* Issue Program */
    mov     r2, #0x40
    strb    r2, [r0]
    strh    r1, [r0]

    /* Wait */
1:  ldrh    r1, [r0]
    mov     r2, #0x80
    and     r1, r2
    beq     1b

    /* Read status */
    ldrh    r1, [r0]
    mov     r2, #0x10
    and     r1, r2

    /* Issue Read Array */
    mov     r2, #0xff
    strb    r2, [r0]
    mov     r0, r1
    bx      lr

