CC=avr-gcc
LD=avr-ld
CCFLAGS=-Wall -Os -mcall-prologues -DF_CPU=16000000UL -DSOCK_NO_CLIENT -DSUPPORT_MCP9800 -DSUPPORT_MMC -DDEBUG
OBJCOPY=avr-objcopy
MCU=atmega328p
#MCU=atmega88p
LDFLAGS=

all: main.hex

objects:=$(patsubst %.c,%.o,$(wildcard *.c)) romfs_text.o
-include $(objects:.o=.d)

fuse:
	avrdude -cusbtiny -pm328p -v -U lfuse:w:0xf7:m

program: main.hex
	#avrdude -carduino -P /dev/ttyUSB0 -b57600 -pm328p -v -U flash:w:$<
	#avrdude -cusbtiny -pm88p -v -U flash:w:$<
	avrdude -cusbtiny -pm328p -v -U flash:w:$<

main.elf: $(objects)
	$(CC) $(CCFLAGS) -mmcu=$(MCU) -o $@ $^

romfs.bin:
	@echo romfs.bin: `find htdocs -name .svn -prune -o -print | paste -s` > romfs_text.d
	./buildfs.py > $@

romfs_text.o: romfs.bin
	$(LD) $(LDFLAGS) -b binary $< -o $@
	$(OBJCOPY) --redefine-sym _binary_romfs_bin_start=romfs_text --rename-section .data=.progmem $@

%.hex: %.elf
	$(OBJCOPY) -O ihex $< $@
	avr-size -C --mcu=$(MCU) $<	

.c.o: %.h
	$(CC) $(CCFLAGS) -MMD -mmcu=$(MCU) -c -o $@ $<

clean:
	rm -f romfs.bin
	rm -f *.o
	rm -f *.d
	rm -f *.elf
	rm -f *.hex
