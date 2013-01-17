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
    nop                     /* +4: Blob checksum */
entry:                      /* +8: ROM jumps here */
    nop
    bl      start

/*.balign 0x30
    .ascii  ""              / * +30: Build timestamp * /
.balign 0x50
    .ascii  ""              / * +50: Comment * /
*/

/* The real entry point. Jump to the "real" firmware */
/* but initialize our memory first. */
.arm
.balign 0x80                /* +80: Entry point */
start:
    ldr     r0, =(thumb+1)
    bx      r0
.thumb
thumb:
    /* Disable interrupts? */
    ldr     r0, =0x00800500
    mov     r1, #0
    mvn     r1, r1
    str     r1, [r0, #8]
    ldr     r0, =0x00800600
    str     r1, [r0, #8]

    /* Setup temporary stack */
    ldr     r0, =0x2600
    mov     sp, r0

    /* Check for firmware upgrade */
    bl      TryFirmwareUpgrade

    /* Initialize backpack */
    bl      InitDataZeroBSS

    /* Call original firmware */
    ldr     r0, =arm
    bx      r0
.arm
arm:
    ldr     r0, =0x01010004
    mov     pc, r0

