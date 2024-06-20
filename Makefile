#
#       !!!! Do NOT edit this makefile with an editor which replace tabs by spaces !!!!
#
##############################################################################################
#
# On command line:
#
# make all = Create project
#
# make clean = Clean project files.
#
# To rebuild project do "make clean" and "make all".
#
# Included originally in the yagarto projects. Original Author : Michael Fischer
# Modified to suit our purposes by Hussam Al-Hertani
# Use at your own risk!!!!!
##############################################################################################
# Start of default section
#
TRGT = arm-none-eabi-
CC   = $(TRGT)gcc
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
HEX  = $(CP) -O ihex
BIN  = $(CP) -O binary -S
MCU  = cortex-m0plus

# List all default C defines here, like -D_DEBUG=1
# for processor STM32F051C8T6
# DDEFS = -DSTM32F051
# for processor STM32F030K6T6
# DDEFS = -DSTM32F030
# for processor STM32G030K8T6, STM32G030J6M6
DDEFS = -DSTM32G030xx
# for processor STM32G070KBT6
# DDEFS = -DSTM32G070xx


# List all default directories to look for include files here
DINCDIR = ./src

# List the default directory to look for the libraries here
DLIBDIR =

# List all default libraries here
DLIBS =

#
# End of default section
##############################################################################################

##############################################################################################
# Start of user section
#

#
# Define project name, check on linker selection part
# PROJECT = Template_G030

# List all user C define here, like -D_DEBUG=1
UDEFS =

# Define ASM defines here
UADEFS =

# List C source files here
CORELIBDIR = ./cmsis_core
BOOTDIR = ./cmsis_boot

LINKER = ./cmsis_boot/startup

SRC  = ./src/main.c
SRC += $(BOOTDIR)/system_stm32g0xx.c
SRC += $(BOOTDIR)/syscalls/syscalls.c

SRC += ./src/it.c
SRC += ./src/gpio.c
SRC += ./src/tim.c
SRC += ./src/adc.c
SRC += ./src/dma.c
SRC += ./src/usart.c
SRC += ./src/flash_program.c
SRC += ./src/hard.c
SRC += ./src/test_functions.c

SRC += ./src/lcd.c
SRC += ./src/lcd_utils.c
SRC += ./src/dmx_receiver.c
SRC += ./src/temperatures.c
SRC += ./src/dmx_mode.c
SRC += ./src/menues.c
SRC += ./src/manual_mode.c
SRC += ./src/dmx_lcd_menu.c
SRC += ./src/dmx_utils.c
SRC += ./src/comms_power.c

SRC += ./src/dsp.c



## Core Support
# SRC += $(CORELIBDIR)/core_cm0.c


## used part of GSM_Library
#SRC += $(DLIBDIR)/src/at_parser.c
#SRC += $(DLIBDIR)/src/at_timer.c
#SRC += $(DLIBDIR)/src/gsm_adapter.c
#SRC += $(DLIBDIR)/src/gsm_engine.c
#SRC += $(DLIBDIR)/src/gsm_hal.c


# List ASM source files here, check linker part selection
# ASRC = ./cmsis_boot/startup/startup_stm32g030xx.s

# List all user directories here
UINCDIR = $(BOOTDIR) \
          $(CORELIBDIR)
	#../paho.mqtt.embedded-c/MQTTPacket/src

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS =

# Define optimisation level here
# O1 optimiza size no significativo
# O2 size mas fuerte
# Os (size mas fuerte que O2)
# O3 el mas fuerte, seguro despues no corre
# O0 (no optimiza casi nada, mejor para debug)
OPT = -O0

#
# End of user defines
##############################################################################################
#
# Define linker script file here
#
ifeq ($(DDEFS), -DSTM32G070xx)
$(info --------------- STM32G070 VERSION ---------------)
PROJECT = Template_G070
ASRC = ./cmsis_boot/startup/startup_stm32g070xx.s
LDSCRIPT = $(LINKER)/stm32_flash_g070.ld
else
$(info --------------- STM32G030 VERSION ---------------)
PROJECT = Template_G030
ASRC = ./cmsis_boot/startup/startup_stm32g030xx.s
LDSCRIPT = $(LINKER)/stm32_flash_g030.ld
endif

FULL_PRJ = $(PROJECT)_rom

INCDIR  = $(patsubst %,-I%,$(DINCDIR) $(UINCDIR))
LIBDIR  = $(patsubst %,-L%,$(DLIBDIR) $(ULIBDIR))

ADEFS   = $(DADEFS) $(UADEFS)

LIBS    = $(DLIBS) $(ULIBS)
MCFLAGS = -mcpu=$(MCU)

ASFLAGS = $(MCFLAGS) -g -gdwarf-2 -mthumb  -Wa,-amhls=$(<:.s=.lst) $(ADEFS)

# SIN INFO DEL DEBUGGER
#CPFLAGS = $(MCFLAGS) $(OPT) -gdwarf-2 -mthumb   -fomit-frame-pointer -Wall -Wstrict-prototypes -fverbose-asm -Wa,-ahlms=$(<:.c=.lst) $(DEFS)

# CON INFO PARA DEBUGGER
#CPFLAGS = $(MCFLAGS) $(OPT) -g -gdwarf-2 -mthumb -fomit-frame-pointer -Wall -fverbose-asm -Wa,-ahlms=$(<:.c=.lst) $(DEFS)

# CON INFO PARA DEBUGGER + STRIP CODE
CPFLAGS = $(MCFLAGS) $(OPT) -g -gdwarf-2 -mthumb -fomit-frame-pointer -Wall -fdata-sections -ffunction-sections -fverbose-asm -Wa,-ahlms=$(<:.c=.lst) $(DDEFS)

# SIN DEAD CODE, hace el STRIP
LDFLAGS = $(MCFLAGS) -mthumb --specs=nano.specs -Wl,--gc-sections -nostartfiles -T$(LDSCRIPT) -Wl,-Map=$(FULL_PRJ).map,--cref,--no-warn-mismatch $(LIBDIR)
# CON DEAD CODE
#LDFLAGS = $(MCFLAGS) -mthumb --specs=nano.specs -nostartfiles -T$(LDSCRIPT) -Wl,-Map=$(FULL_PRJ).map,--cref,--no-warn-mismatch $(LIBDIR)
#LDFLAGS = $(MCFLAGS) -mthumb -T$(LDSCRIPT) -Wl,-Map=$(FULL_PRJ).map,--cref,--no-warn-mismatch $(LIBDIR)

#
# makefile rules
#

assemblersources = $(ASRC)
sources = $(SRC)
OBJS  = $(SRC:.c=.o) $(ASRC:.s=.o)

objects = $(sources:.c=.o)
assobjects = $(assemblersources:.s=.o)

all: $(objects) $(assobjects) $(FULL_PRJ).elf $(FULL_PRJ).bin
	arm-none-eabi-size $(FULL_PRJ).elf
	gtags -q

$(objects): %.o: %.c
	$(CC) -c $(CPFLAGS) -I. $(INCDIR) $< -o $@

$(assobjects): %.o: %.s
	$(AS) -c $(ASFLAGS) $< -o $@

%elf: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(LIBS) -o $@

%hex: %elf
	$(HEX) $< $@

%bin: %elf
	$(BIN)  $< $@

flash:
ifeq ($(DDEFS), -DSTM32G070xx)
	sudo openocd -f stm32g070_flash.cfg
else
	sudo openocd -f stm32g030_flash.cfg
endif


flash_lock:
	sudo openocd -f stm32g0_flash_lock.cfg

mass_erase:
	sudo openocd -f stm32g0_flash_erase.cfg

gdb:
	sudo openocd -f stm32g0_gdb.cfg

reset:
	sudo openocd -f stm32g0_reset.cfg

clean:
	rm -f $(OBJS)
	rm -f $(FULL_PRJ).elf
	rm -f $(FULL_PRJ).map
	rm -f $(FULL_PRJ).hex
	rm -f $(FULL_PRJ).bin
	rm -f $(SRC:.c=.lst)
	rm -f $(ASRC:.s=.lst)
	rm -f *.o
	rm -f *.out
	rm -f *.gcda
	rm -f *.gcno
	rm -f *.gcov


tests:
	# simple functions tests, copy functions to tests module into main
	gcc src/tests.c
	./a.out


tests_comms_power:
	# first module objects to test
	gcc -c src/comms_power.c -I. $(INCDIR) $(DDEFS)
	# second auxiliary helper modules
	gcc -c src/tests_ok.c -I $(INCDIR)
	gcc -c src/tests_mock_usart.c -I $(INCDIR)
	gcc src/tests_comms_power.c comms_power.o tests_ok.o tests_mock_usart.o -I $(INCDIR) $(DDEFS)
	./a.out


tests_oled_screen:
	# first compile common modules (modules to test and dependencies)
	gcc -c src/screen.c -I. $(INCDIR)
	gcc -c src/ssd1306_display.c -I. $(INCDIR)
	gcc -c src/ssd1306_gfx.c -I. $(INCDIR)
	# the module that implements tests_lcd_application.h functions
	gcc -c `pkg-config --cflags gtk+-3.0` src/tests_oled_app.c -o tests_oled_app.o
	# then the gtk lib modules
	gcc -c `pkg-config --cflags gtk+-3.0` src/tests_glade_oled.c -o tests_glade_oled.o
	# link everything
	gcc tests_glade_oled.o tests_oled_app.o screen.o ssd1306_display.o ssd1306_gfx.o `pkg-config --libs gtk+-3.0` -o tests_gtk
	# run global tags
	gtags -q
	# run the simulation
	# ./tests_gtk


# *** EOF ***
