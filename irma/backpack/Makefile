TOOLCHAIN = arm-none-eabi
CC = $(TOOLCHAIN)-gcc
LD = $(TOOLCHAIN)-ld
OBJCOPY = $(TOOLCHAIN)-objcopy
NM = $(TOOLCHAIN)-nm
LDNAME = irma_lo.ld
CFLAGS =-mcpu=arm7tdmi -mthumb -I. -I./lwbt/ -I ./fulibc/inc/ -I ./utils/ -I ./app/ -std=gnu99 -nostdlib -nostdinc -fno-builtin -Wcast-align -g

LWBT = lwbt/fcs.o lwbt/hci.o lwbt/l2cap.o lwbt/lwbt_memp.o lwbt/rfcomm.o lwbt/sdp.o
LWIP = lwip/mem.o lwip/memp.o lwip/pbuf.o lwip/inet.o lwip/stats.o
FULIBC = $(subst .c,.o,$(shell find fulibc/ -name '*.c'))
APP = $(subst .c,.o,$(shell find app/ -name '*.c'))
UTILS = utils/utils.o utils/uart.o
HOST = host/api.o host/head.o host/init.o

all: backpack.elf

clean:
	rm -rf *.o *.elf *.bin ${LWBT} ${LWIP} ${FULIBC} ${APP} ${HOST} ${UTILS}

backpack.elf: ${APP} ${HOST} ${UTILS} ${LWBT} ${LWIP} ${FULIBC}
	${LD} -T ${LDNAME} -g -o $@ $^
	${OBJCOPY} -O binary $@ ${@:.elf=.bin}
	${NM} -n $@ > ${@:.elf=.sym}

