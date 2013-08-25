TOOLCHAIN = arm-none-eabi
CC = $(TOOLCHAIN)-gcc
LD = $(TOOLCHAIN)-ld
OBJCOPY = $(TOOLCHAIN)-objcopy
NM = $(TOOLCHAIN)-nm
LDNAME = ../common/irma_hi.ld
CFLAGS =-mcpu=arm7tdmi -mthumb -I../../irma/backpack/ -I../../irma/backpack/lwbt/ -I ../../irma/backpack/fulibc/inc/ -I ../../irma/backpack/utils/ -I ../../irma/backpack/app/ -std=gnu99 -nostdlib -nostdinc -fno-builtin -Wcast-align -g -D__IRMA_FLAGS__=1

TARGET=nolleblink13
OBJS=head.o main.o bsl.o spp.o

all: ${TARGET}.elf

clean:
	rm -rf *.o ${TARGET}.elf *.bin ${OBJS}

head.o: ../common/head.S
	${CC} -c ${CFLAGS} -o $@ $^

${TARGET}.elf: ${OBJS} 
	${LD} -T ${LDNAME} -g -R ../common/libackpack.elf -o $@  $^ 
	${OBJCOPY} -O binary $@ ${@:.elf=.bin}
	${NM} -n $@ > ${@:.elf=.sym}
	../common/bless.py ${@:.elf=.bin}

