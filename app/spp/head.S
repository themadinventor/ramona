/*
 * Ericsson Baseband Controller
 * Plugin Header
 *
 * 2013-01-08 <fredrik@etf.nu>
 */

.thumb
.section .head

/* Plugin Header */
magic:
    .int    0x414d5249      /*  +0: Magic */
    .int    0               /*  +4: Flags */
    .int    _text           /*  +8: .text */
    .int    _etext          /*  +c: end of .text */
    .int    _data           /* +10: .data */
    .int    _bss            /* +14: end of data, .bss */
    .int    _ebss           /* +18: end of .bss */
    .int    0               /* +1c: Checksum */
entry:                      /* +20: Entry point */
    b       start
