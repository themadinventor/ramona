#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "uart.h"

volatile uint8_t led_state = 0;

ISR(TIMER0_COMPA_vect)
{
    if (led_state == 2) {
        PORTD |= _BV(5);
        led_state = 3;
    } else if (led_state == 3) {
        PORTD &= ~_BV(5);
        led_state = 0;
    }
}

int main(void)
{
    /* Status LED */
    DDRD = _BV(5);

    /* UART to IRMA */
    uart_init();

    /* Internal timekeeping, 100Hz */
    TCCR0A = _BV(WGM01);
    TCCR0B = _BV(CS02)|_BV(CS00);
    OCR0A = 127;
    TIMSK0 = _BV(OCIE0A);

    /* Go for it! */
    sei();

    while (1) {
        if (uart_rxlen() > 0) {
            char c = uart_receive();
        
            if (c == 0x01) {
                PORTD |= _BV(5);
                led_state = 1;
            } else if (c == 0x02) {
                PORTD &= ~_BV(5);
                led_state = 0;
            } else if (c == 0x03) {
                led_state = 2;
            }
        }
    };
}

