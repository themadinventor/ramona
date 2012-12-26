/*
 * Ericsson Baseband Controller
 * IRMA Backpack Memory Initialization
 *
 * 2012-12-25 <fredrik@etf.nu>
 */

/*
 * vanilla bss at 00000e74 and 54548 bytes long:
 * bss ends at 0000e388
 * 
 * our bss at 0000e400 until end of space and time (15 kB-ish)
 */

extern void _text, _etext, _data, _edata, _bss, _ebss;

/*
 * Initialize the data segment from flash and clear the .bss seg.
 */
void InitDataZeroBSS(void)
{
    register char *src = &_etext;
    register char *dst = &_data;
    register char *end = &_edata;

    while (dst < end)
        *dst++ = *src++;

    for (dst = &_bss, end = &_ebss; dst < end; dst++)
        *dst = 0;
}

