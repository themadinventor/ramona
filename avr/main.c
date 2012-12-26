#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "uart.h"

int main(void)
{
    /* Status LED */
    DDRD = _BV(5);

    uart_init();

    /* Go for it! */
    sei();

    while (1) {
        if (uart_rxlen() > 0) {
            char c = uart_receive();
        
            if (c == 0x01) {
                PORTD |= _BV(5);
            } else if (c == 0x02) {
                PORTD &= ~_BV(5);
            }
        }
    };
}

