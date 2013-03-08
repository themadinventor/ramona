MEMORY
{
    FLASH (rx) : ORIGIN = 0x01060000, LENGTH = 0x00090000
    RAM (rwx)  : ORIGIN = 0x00011000, LENGTH = 0x00001000
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
