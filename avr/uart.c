#include <avr/io.h>
#include <avr/interrupt.h>
#include "fifo.h"

FIFO_DEF(uart_rx);
FIFO_DEF(uart_tx);

ISR(USART_RX_vect)
{
    uint8_t data = UDR0;
    sei();

    FIFO_PUT(uart_rx, data);
}

ISR(USART_UDRE_vect)
{
    if (FIFO_EMPTY(uart_tx)) {
        UCSR0B &= ~_BV(UDRIE0);
        return;
    }

    UDR0 = FIFO_GET(uart_tx);
}

static inline void uart_configure(long baud)
{
    uint16_t ubrr = (F_CPU / 16) / baud - 1;

    UBRR0H = ubrr >> 8;
    UBRR0L = ubrr;
}

void uart_init(void)
{
    UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
    
    uart_configure(115200);
}

void uart_transmit(uint8_t c)
{
    FIFO_PUT(uart_tx, c);
    UCSR0B |= _BV(UDRIE0);
}

uint8_t uart_rxlen(void)
{
    return FIFO_LENGTH(uart_rx);
}

uint8_t uart_receive(void)
{
    if (FIFO_EMPTY(uart_rx)) {
        return 0xff;
    }

    return FIFO_GET(uart_rx);
}

