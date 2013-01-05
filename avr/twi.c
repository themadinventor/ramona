#include <util/twi.h>

/*ISR(TWI_vect)
{
    switch (TWSR) {
        case TW_ST_SLA_ACK:

        case TW_ST_DATA_ACK:
            TWDR = 0x55;
            TWCR = _BV(TWEN)|_BV(TWIE)|_BV(TWINT)|_BV(TWEA);
            break;

        case TW_ST_DATA_NACK:
            break;

        case TW_SR_SLA_ACK: // our address, ack
            PORTD |= _BV(5);
            TWCR = _BV(TWEN)|_BV(TWIE)|_BV(TWINT)|_BV(TWEA);
            TWDR = 0x33;
            break;

        case TW_SR_DATA_ACK: // data while addressed, ack
            { char c = TWDR; }
            TWDR = 0xcc;
            TWCR = _BV(TWEN)|_BV(TWIE)|_BV(TWINT)|_BV(TWEA);
            break;

        case TW_SR_STOP:
            break;
    }
}*/

    /* I2C Bus to IRMA */
    PORTC = _BV(4)|_BV(5); // Pull-up on SDA and SCL

    /*TWAR = (1 << 1);
    TWDR = 0xFF;
    TWCR = _BV(TWEN)|_BV(TWIE)|_BV(TWINT)|_BV(TWEA);*/
