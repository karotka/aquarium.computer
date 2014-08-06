PRG            = main
OBJ            = main.o twimaster.o uart.o rtc8563.o # adc.o
MCU_TARGET     = atmega328p
OPTIMIZE       = -O2
ISP            = usbtiny
F_CPU          = 16000000

# You should not have to change anything below here.

CC             = avr-gcc

# Override is only needed by avr-lib build system.

override CFLAGS =  -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS) -DF_CPU=$(F_CPU)

OBJCOPY        = avr-objcopy
OBJDUMP        = avr-objdump

all: hex

$(PRG).elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -rf *.o $(PRG).elf $(PRG).hex

hex:  $(PRG).hex

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

install: load

load: $(PRG).hex
	avrdude -P usb -p m328p -c $(ISP) -U flash:w:$<

status:
	avrdude -P usb -p m328p -c $(ISP) -v
