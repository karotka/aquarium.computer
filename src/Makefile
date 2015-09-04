PRG            = main
OBJ            = main.o twimaster.o rtc8563.o adc.o uart.o fifo.o
MCU_TARGET     = atmega328p
OPTIMIZE       = -O2
ISP            = usbtiny
F_CPU          = 16000000

# You should not have to change anything below here.

CC             = avr-gcc

# Override is only needed by avr-lib build system.

override CFLAGS  =  -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS) -DF_CPU=$(F_CPU)
override LDFLAGS =

OBJCOPY        = avr-objcopy
OBJDUMP        = avr-objdump

all: hex

$(PRG).elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS) -lm

clean:
	rm -rf *.o *.elf *.hex

hex:  $(PRG).hex

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

install: load

test:
	$(CC) $(CFLAGS) $(LDFLAGS) -c -o uart.o  uart.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c -o adc.o  adc.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c -o test.o  test.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o test.elf test.o uart.o adc.o -lm
	$(OBJCOPY) -j .text -j .data -O ihex test.elf test.hex
	avrdude -P usb -p m328p -c $(ISP) -U flash:w:test.hex

load: $(PRG).hex
	avrdude -P usb -p m328p -c $(ISP) -U flash:w:$<

status:
	avrdude -P usb -p m328p -c $(ISP) -v
