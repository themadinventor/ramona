TOOLCHAIN = arm-none-eabi
CC = $(TOOLCHAIN)-gcc
LD = $(TOOLCHAIN)-ld
OBJCOPY = $(TOOLCHAIN)-objcopy
NM = $(TOOLCHAIN)-nm
LDNAME = irma_hi.ld
CFLAGS =-mcpu=arm7tdmi -mthumb -I../../irma/backpack/ -I../../irma/backpack/lwbt/ -I ../../irma/backpack/fulibc/inc/ -I ../../irma/backpack/utils/ -I ../../irma/backpack/app/ -std=gnu99 -nostdlib -nostdinc -fno-builtin -Wcast-align -g

TARGET=cputest
OBJS=head.o main.o

all: ${TARGET}.elf

clean:
	rm -rf *.o ${TARGET}.elf *.bin ${OBJS}

${TARGET}.elf: ${OBJS} 
	${LD} -T ${LDNAME} -g -R libackpack.elf -o $@  $^ 
	${OBJCOPY} -O binary $@ ${@:.elf=.bin}
	${NM} -n $@ > ${@:.elf=.sym}
	./bless.py ${@:.elf=.bin}

