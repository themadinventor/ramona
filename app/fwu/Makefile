TOOLCHAIN = arm-none-eabi
CC = $(TOOLCHAIN)-gcc
LD = $(TOOLCHAIN)-ld
OBJCOPY = $(TOOLCHAIN)-objcopy
NM = $(TOOLCHAIN)-nm
LDNAME = irma_fwu.ld
CFLAGS =-mcpu=arm7tdmi -mthumb -std=gnu99 -nostdlib -nostdinc -fno-builtin -Wcast-align -g

TARGET=fwu
OBJS=head.o main.o flash.o uart.o utils.o rom.o

all: ${TARGET}.elf

clean:
	rm -rf *.o ${TARGET}.elf *.bin ${OBJS}

${TARGET}.elf: ${OBJS} 
	${LD} -T ${LDNAME} -g -o $@  $^ 
	${OBJCOPY} -O binary $@ ${@:.elf=.bin}
	${NM} -n $@ > ${@:.elf=.sym}
	./bless.py ${@:.elf=.bin}

