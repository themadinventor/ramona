/*
 * Ericsson Baseband Controller
 * FWU Header
 *
 * 2013-01-09 <fredrik@etf.nu>
 */

.thumb
.section .head

/* Plugin Header */
magic:
    .int    0x44465755      /*  +0: Magic */
    .int    0               /*  +4: Flags */
    .int    _text           /*  +8: .text */
    .int    _etext          /*  +c: end of .text */
    .int    _data           /* +10: .data */
    .int    _bss            /* +14: end of data, .bss */
    .int    _ebss           /* +18: end of .bss */
    .int    0               /* +1c: Checksum */
entry:                      /* +20: Entry point */
    ldr     r3, =(start+1)
    bx      r3
