TARGET=binaryclock

OUTDIR=build
CFILES=main.c uart.c rtc.c sendbyte.S binary_display.c button.c
HFILES=uart.h rtc.h sendbyte.h binary_display.h button.h
OPTIONS= -O2 -DF_CPU=16000000 -Wall -Wextra # -DDEBUG

DEVICE=atmega328p
PROGRAMMER=avrispmkII
#PROGRAMMER=stk500v2

$(OUTDIR)/$(TARGET): $(CFILES) $(HFILES)
	if [ ! -d "$(OUTDIR)" ]; then mkdir $(OUTDIR); fi
	avr-gcc $(OPTIONS) -mmcu=$(DEVICE) -o $(OUTDIR)/$(TARGET) $(CFILES)
	avr-objcopy -j .text -j .data -O ihex $(OUTDIR)/$(TARGET) $(OUTDIR)/$(TARGET).hex
	avr-objcopy -j .text -j .data -O binary $(OUTDIR)/$(TARGET) $(OUTDIR)/$(TARGET).bin

flash: $(OUTDIR)/$(TARGET)
	sudo avrdude -p $(DEVICE) -c $(PROGRAMMER) -U flash:w:$(OUTDIR)/$(TARGET).hex:i -F -P usb
	#sudo avrdude -p $(DEVICE) -c $(PROGRAMMER) -U flash:w:$(TARGET).hex:i -F -P /dev/ttyUSB0

# sudo avrdude -P usb -c avrispmkII -p atmega1284p -v -U lfuse:w:0xDE:m   		// writes 0xDE lfuse
# https://eleccelerator.com/fusecalc/fusecalc.php
