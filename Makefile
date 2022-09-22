CC := avr-gcc
CC_FLAGS := -O0 -w -std=c99	#disable optimization. disable warnings. compiler switch ISO-C
MCU := atmega32
SRC_FILE := main
COMPILE := $(CC) $(CC_FLAGS) -mmcu=$(MCU)
EXECUTABLE := project2

default: compile

compile:
	@echo "Compiling.." 
	@$(COMPILE) -c $(SRC_FILE).c -o $(EXECUTABLE).o	
	@$(COMPILE) -o $(EXECUTABLE).elf $(EXECUTABLE).o
	@avr-objcopy -j .text -j .data -O ihex $(EXECUTABLE).elf $(EXECUTABLE).hex
	@echo "Program Size"
	avr-size --format=avr --mcu=$(MCU) $(EXECUTABLE).elf

clean:
	@echo "removing compilation files.." 
	@rm $(EXECUTABLE).elf $(EXECUTABLE).o $(EXECUTABLE).hex
