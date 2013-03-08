#ifndef PROTO_H
#define PROTO_H

/*
 *  [5 bit opcode] [3 bit arg]
 */

#define PROTO_CMD(OP, ARG)  (((OP) << 3) | (ARG))
#define PROTO_OP(X)         (((X) >> 3) & 0x1f)
#define PROTO_ARG(X)        ((X) & 0x07)

enum {
    GPIO_SET_PORT,
    GPIO_SET_DDR,
    GPIO_STROBE_PORT,
    GPIO_STROBE_DDR,
    GPIO_READ_PIN,

    EEPROM_ADDR,
    EEPROM_READ,
    EEPROM_WRITE,

    SPI_CONFIG,
    SPI_XCHG,

    ADC_CONFIG,
    ADC_READ,
    ADC_READL,

    REG_ADDR,
    REG_READ,
    REG_WRITE,

    FW_VERSION,
    FW_BOOTLOADER
};

enum {
    GPIO_PORTB,
    GPIO_PORTC,
    GPIO_PORTD
};

enum {
    ADC_0,
    ADC_1,
    ADC_2,
    ADC_3,
    ADC_4,
    ADC_5,
    ADC_6,
    ADC_7
};

#endif
