#ifndef FIFO_H
#define FIFO_H

#define FIFO_DEF(NAME) volatile uint8_t NAME ## _buf[64], NAME ## _head, NAME ## _tail;

#define FIFO_PUT(NAME, DATA) { NAME ## _head = (NAME ## _head + 1) & 0x3f; NAME ## _buf[NAME ## _head] = (DATA); }

#define FIFO_GET(NAME) (NAME ## _tail = (NAME ## _tail + 1) & 0x3f, NAME ## _buf[NAME ## _tail])

#define FIFO_EMPTY(NAME) (NAME ## _tail == NAME ## _head)

#define FIFO_LENGTH(NAME) (NAME ## _head - NAME ## _tail + ((NAME ## _head >= NAME ## _tail) ? 0 : 64))

#endif
