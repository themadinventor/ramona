MEMORY
{
    FLASH (rx) : ORIGIN = 0x01060000, LENGTH = 0x00090000
    RAM (rwx)  : ORIGIN = 0x00002600, LENGTH = 0x0000fa00
}

SECTIONS
{
    .text :
    {
        _text = .;
        KEEP(*(.head))
        _etext = .;
    } > FLASH
    .data :
    AT ( ADDR (.text) + SIZEOF (.text) )
    {
        _data = .;
        *(.text*)
        *(.rodata*)
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
