#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
    DDRD = _BV(5);

    while (1) {
        PORTD |= _BV(5);
        _delay_ms(50);

        PORTD &= ~_BV(5);
        _delay_ms(450);
    };
}

