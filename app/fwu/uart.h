/*
 * Ericsson Baseband Controller
 * UART Driver
 *
 * 2012-07-25 <fredrik@etf.nu>
 */

#ifndef __UART_H__
#define __UART_H__

extern void UARTInit(void);
/*extern void UART1SetBaudRate(unsigned char divisor);
extern void UART1WriteString(const char *);
extern char UART1GetChar(void);
extern void UART1PutChar(char);
extern char UART1ReadByte(void);
extern void UART1WriteByte(char);
extern int UART1GetTxFIFOSize(void);*/

extern void UART2SetBaudRate(unsigned char divisor);
extern void UART2WriteString(const char *);
extern char UART2GetChar(void);
extern void UART2PutChar(char);
extern char UART2ReadByte(void);
extern void UART2WriteByte(char);
extern int UART2GetTxFIFOSize(void);

/*extern void UART3SetBaudRate(unsigned char divisor);
extern void UART3WriteString(const char *);
extern char UART3GetChar(void);
extern void UART3PutChar(char);
extern char UART3ReadByte(void);
extern void UART3WriteByte(char);*/

#define UART_460800 0x00
#define UART_230400 0x01
#define UART_115200 0x02
#define UART_57600  0x03
#define UART_28800  0x04
#define UART_14400  0x05
#define UART_7200   0x06
#define UART_3600   0x07
#define UART_1800   0x08
#define UART_900    0x09
#define UART_153600 0x10
#define UART_76800  0x11
#define UART_38400  0x12
#define UART_19200  0x13
#define UART_9600   0x14
#define UART_4800   0x15
#define UART_2400   0x16
#define UART_1200   0x17

#endif
