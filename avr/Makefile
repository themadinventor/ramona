TARGET  =ramona_avr
MCU     =atmega88p
OBJS    =main.o

CC      =avr-gcc
OBJCOPY =avr-objcopy
SIZE    =avr-size

CFLAGS  =-g -Os -mmcu=$(MCU) -I. -Wall

all: $(TARGET).hex

flash: $(TARGET).hex
	avrdude -c avrftdi -p$(MCU) -U flash:w:$^

size: $(OBJS)
	$(SIZE) $(OBJS)

clean:
	rm -f *.o

veryclean: clean
	rm -f $(TARGET).hex

$(TARGET).hex: $(TARGET).o
	$(OBJCOPY) -j .text -j .data -O ihex $(TARGET).o $@

$(TARGET).o: $(OBJS)
	$(CC) -g -mmcu=$(MCU) -Wall -o $@ $^
	$(SIZE) $@
