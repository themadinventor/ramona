CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy
LDNAME = backpack.ld
CFLAGS =-mcpu=arm7tdmi -mthumb -I. -I./lwbt/ -I ./fulibc/inc/ -I ./utils/ -I ./app/ -std=gnu99 -nostdlib -nostdinc -fno-builtin -Wcast-align -g

LWBT = lwbt/fcs.o lwbt/hci.o lwbt/l2cap.o lwbt/lwbt_memp.o lwbt/rfcomm.o lwbt/sdp.o
LWIP = lwip/mem.o lwip/memp.o lwip/pbuf.o lwip/inet.o lwip/stats.o
FULIBC = $(subst .c,.o,$(shell find fulibc/ -name '*.c'))
APP = $(subst .c,.o,$(shell find app/ -name '*.c'))
UTILS = utils/utils.o utils/math.o utils/uart.o
HOST = host/api.o host/head.o

all: backpack.elf

clean:
	rm -rf *.o *.elf *.bin ${LWBT} ${LWIP} ${FULIBC}

backpack.elf: ${APP} ${HOST} ${UTILS} ${LWBT} ${LWIP} ${FULIBC}
	${LD} -T ${LDNAME} -g -o $@ $^
	${OBJCOPY} -O binary $@ ${@:.elf=.bin}
