/*
 * Ericsson Baseband Controller
 * IRMA definitions
 *
 * 2013-07-14 <fredrik@etf.nu>
 */

#ifndef __IRMA_H__
#define __IRMA_H__

#include <stdint.h>

/* Memory map */
#define RAM_BASE    0x00000000
#define ROM_BASE    0x00c00000
#define EMIF0_BASE  0x01000000
#define EMIF1_BASE  0x01400000
#define EMIF2_BASE  0x01800000
#define DMA_READ    0x20000000
#define DMA_WRITE   0x30000000

/* Perioherals */
#define UART1_BASE  0x00800000
#define UART2_BASE  0x00800100
#define UART3_BASE  0x00800200
#define EMIF_BASE   0x00800300
#define GPIO_BASE   0x00800400
#define IRQ_BASE    0x00800500
#define FIQ_BASE    0x00800600
#define TIMER_BASE  0x00800700
#define EBC_BASE    0x00800800
#define CLOCK_BASE  0x00800900
#define I2C_BASE    0x00800a00
#define FIFO0_BASE  0x00800b00
#define FIFO1_BASE  0x00800b10
#define FIFO2_BASE  0x00800b20
#define FIFO3_BASE  0x00800b30
#define WDT_BASE    0x00800c00
#define IO_BASE     0x00800d00
#define USB_BASE    0x00802000

/* UARTs */
#define UART_THR(BASE)  (((uint8_t *)(BASE))[0x00])
#define UART_IER(BASE)  (((uint8_t *)(BASE))[0x04])
#define UART_IIR(BASE)  (((uint8_t *)(BASE))[0x08])
#define UART_FCR(BASE)  (((uint8_t *)(BASE))[0x08])
#define UART_LCR(BASE)  (((uint8_t *)(BASE))[0x0c])
#define UART_MCR(BASE)  (((uint8_t *)(BASE))[0x10])
#define UART_LSR(BASE)  (((uint8_t *)(BASE))[0x14])
#define UART_MSR(BASE)  (((uint8_t *)(BASE))[0x18])
#define UART_SCR(BASE)  (((uint8_t *)(BASE))[0x1c])

#define UART1_THR   UART_THR(UART1_BASE)
#define UART1_IER   UART_IER(UART1_BASE)
#define UART1_IIR   UART_IIR(UART1_BASE)
#define UART1_LCR   UART_LCR(UART1_BASE)
#define UART1_MCR   UART_MCR(UART1_BASE)
#define UART1_LSR   UART_LSR(UART1_BASE)
#define UART1_MSR   UART_MSR(UART1_BASE)
#define UART1_SCR   UART_SCR(UART1_BASE)

#define UART2_THR   UART_THR(UART2_BASE)
#define UART2_IER   UART_IER(UART2_BASE)
#define UART2_IIR   UART_IIR(UART2_BASE)
#define UART2_LCR   UART_LCR(UART2_BASE)
#define UART2_MCR   UART_MCR(UART2_BASE)
#define UART2_LSR   UART_LSR(UART2_BASE)
#define UART2_MSR   UART_MSR(UART2_BASE)
#define UART2_SCR   UART_SCR(UART2_BASE)

#define UART3_THR   UART_THR(UART3_BASE)
#define UART3_IER   UART_IER(UART3_BASE)
#define UART3_IIR   UART_IIR(UART3_BASE)
#define UART3_FCR   UART_FCR(UART3_BASE)
#define UART3_LCR   UART_LCR(UART3_BASE)
#define UART3_MCR   UART_MCR(UART3_BASE)
#define UART3_LSR   UART_LSR(UART3_BASE)
#define UART3_MSR   UART_MSR(UART3_BASE)
#define UART3_SCR   UART_SCR(UART3_BASE)

/* FIFOs */
#define FIFO_DATA(BASE)     (((uint8_t *)(BASE))[0x00])
#define FIFO_WTF(BASE)      (((uint8_t *)(BASE))[0x04])
#define FIFO_CONTROL(BASE)  (((uint8_t *)(BASE))[0x08])
#define FIFO_STATUS(BASE)   (((uint8_t *)(BASE))[0x0c])

#define FIFO0_DATA      FIFO_DATA(FIFO0_BASE)
#define FIFO0_WTF       FIFO_WTF(FIFO0_BASE)
#define FIFO0_CONTROL   FIFO_CONTROL(FIFO0_BASE)
#define FIFO0_STATUS    FIFO_STATUS(FIFO0_BASE)

#define FIFO1_DATA      FIFO_DATA(FIFO1_BASE)
#define FIFO1_WTF       FIFO_WTF(FIFO1_BASE)
#define FIFO1_CONTROL   FIFO_CONTROL(FIFO1_BASE)
#define FIFO1_STATUS    FIFO_STATUS(FIFO1_BASE)

#define FIFO2_DATA      FIFO_DATA(FIFO2_BASE)
#define FIFO2_WTF       FIFO_WTF(FIFO2_BASE)
#define FIFO2_CONTROL   FIFO_CONTROL(FIFO2_BASE)
#define FIFO2_STATUS    FIFO_STATUS(FIFO2_BASE)

#define FIFO3_DATA      FIFO_DATA(FIFO3_BASE)
#define FIFO3_WTF       FIFO_WTF(FIFO3_BASE)
#define FIFO3_CONTROL   FIFO_CONTROL(FIFO3_BASE)
#define FIFO3_STATUS    FIFO_STATUS(FIFO3_BASE)

/* Watchdog Timer */
#define WDT_CONTROL     (*((uint8_t *) (WDT_BASE + 0x0c)))
#define WDT_RESET       (*((uint8_t *) (WDT_BASE + 0x10)))

#define WDT_PASSWORD    (0xc0)

/* IO */
#define UART1_BAUD      (*((uint8_t *) (IO_BASE + 0x00)))
#define UART_GPIO       (*((uint16_t *) (IO_BASE + 0x04)))
#define UART_MUX        (*((uint16_t *) (IO_BASE + 0x08)))
#define UART2_BAUD      (*((uint8_t *) (IO_BASE + 0x0C)))
#define UART3_BAUD      (*((uint8_t *) (IO_BASE + 0x14)))

#define UART_BAUD_300       (0x19)
#define UART_BAUD_600       (0x18)
#define UART_BAUD_1200      (0x17)
#define UART_BAUD_2400      (0x16)
#define UART_BAUD_4800      (0x15)
#define UART_BAUD_9600      (0x14)
#define UART_BAUD_19200     (0x13)
#define UART_BAUD_38400     (0x12)
#define UART_BAUD_76800     (0x11)
#define UART_BAUD_153600    (0x10)
#define UART_BAUD_900       (0x09)
#define UART_BAUD_1800      (0x08)
#define UART_BAUD_3600      (0x07)
#define UART_BAUD_7200      (0x06)
#define UART_BAUD_14400     (0x05)
#define UART_BAUD_28800     (0x04)
#define UART_BAUD_57600     (0x03)
#define UART_BAUD_115200    (0x02)
#define UART_BAUD_230400    (0x01)
#define UART_BAUD_460800    (0x00)

#define UART_GPIO_LOW       (0)
#define UART_GPIO_HIGH      (1)
#define UART_GPIO_TXD       (2)
#define UART_GPIO_FLOAT     (3)

#define UART1_TXD_LOW       UART_GPIO_LOW
#define UART1_TXD_HIGH      UART_GPIO_HIGH
#define UART1_TXD_TXD       UART_GPIO_TXD
#define UART1_TXD_FLOAT     UART_GPIO_FLOAT
#define UART1_TXD_MASK      UART_GPIO_FLOAT

#define UART2_TXD_LOW       (UART_GPIO_LOW << 2)
#define UART2_TXD_HIGH      (UART_GPIO_HIGH << 2)
#define UART2_TXD_TXD       (UART_GPIO_TXD << 2)
#define UART2_TXD_FLOAT     (UART_GPIO_FLOAT << 2)
#define UART2_TXD_MASK      (UART_GPIO_FLOAT << 2)

#define UART3_TXD_LOW       (UART_GPIO_LOW << 4)
#define UART3_TXD_HIGH      (UART_GPIO_HIGH << 4)
#define UART3_TXD_TXD       (UART_GPIO_TXD << 4)
#define UART3_TXD_FLOAT     (UART_GPIO_FLOAT << 4)
#define UART3_TXD_MASK      (UART_GPIO_FLOAT << 4)

#define UART3_2_MUX         (0x40)

/* ROM */
#define ROM_ASIC_VERSION    (*((uint32_t *)(ROM_BASE + 0x04)))

#define ASIC_IRMAB_P1A      (0x01)
#define ASIC_IRMAB_P2A      (0x02)
#define ASIC_IRMAB_P3A      (0x03)
#define ASIC_IRMAB_P3B      (0x04)
#define ASIC_IRMAB_P4A      (0x05)
#define ASIC_IRMAB_P5A      (0x06)
#define ASIC_IRMAC_P1A      (0x07)
#define ASIC_IRMAB_P4B      (0x08)
#define ASIC_IRMAB_P4C      (0x09)
#define ASIC_IRMAC_P1B      (0x0b)
#define ASIC_IRMAC_P2A      (0x0d)
#define ASIC_BLINK          (0x10)

/* ROM utility functions in IRMA-B P5A (thumb mode) */
#define ROM_CRC16(PTR, LEN, CRC) (*((uint16_t (*)(void*,uint32_t,uint16_t))(ROM_BASE+0x625)))((PTR), (LEN), (CRC))

#endif
