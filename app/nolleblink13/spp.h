#ifndef SPP_H
#define SPP_H

void spp_set_baud(unsigned char baud);
unsigned char spp_get_baud(void);
int spp_init(void);
void spp_teardown(void);

#endif
