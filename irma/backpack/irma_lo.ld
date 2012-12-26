MEMORY
{
    FLASH (rx) : ORIGIN = 0x01000000, LENGTH = 0x00010000
    RAM (rwx)  : ORIGIN = 0x0000e400, LENGTH = 0x00003c00
}

SECTIONS
{
    .text :
    {
        _text = .;
        KEEP(*(.head))
        *(.text*)
        *(.rodata*)
        _etext = .;
    } > FLASH
    .data :
    AT ( ADDR (.text) + SIZEOF (.text) )
    {
        _data = .;
        *(.data*)
        _edata = .;
    } > RAM
    .bss :
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > RAM
}
