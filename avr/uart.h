#ifndef UART_H
#define UART_H

void uart_init(void);
uint8_t uart_rxlen(void);
uint8_t uart_receive(void);
void uart_transmit(uint8_t c);

#endif
