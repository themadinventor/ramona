/*
 * Ericsson Bluetooth Baseband Controller
 * Transport wrapper
 *
 * (c) 2012 <fredrik@z80.se>
 */

#ifndef __TRANSPORT_H__
#define __TRANSPORT_H__

void transport_init(void);
int transport_input(SIGNAL *s);

#endif
