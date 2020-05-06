# Sources

CSRCS = main.c stm32f4xx_it.c system_stm32f4xx.c syscalls.c
CSRCS += controller.c interpreter.c pins.c gpio_mod.c button.c circbuf1.c stasks_mod.c sys_init.c uart_mod.c spi_mod.c ad_mod.c timer_mod.c sin-cos.c

SRCS = $(patsubst %.c, %.o, $(CSRCS))

# USB
SRCS += usbd_usr.c usbd_cdc_vcp.c usbd_desc.c usb_bsp.c

# Project name

#PROJ_NAME=stm32f4_usb_cdc
PROJ_NAME=main

###################################################

# Check for valid float argument
# NOTE that you have to run make clean after
# changing these as hardfloat and softfloat are not
# binary compatible
ifneq ($(FLOAT_TYPE), hard)
ifneq ($(FLOAT_TYPE), soft)
override FLOAT_TYPE = hard
#override FLOAT_TYPE = soft
endif
endif

###################################################

GCC_REL = 5_4-2016q2
#BINPATH=~/gcc-arm-none-eabi-$(GCC_REL)/bin
BINPATH=~/gcc-arm-eabi/bin
CC=$(BINPATH)/arm-none-eabi-gcc
OBJCOPY=$(BINPATH)/arm-none-eabi-objcopy
SIZE=$(BINPATH)/arm-none-eabi-size

CFLAGS  = -std=gnu99 -g -Wall -Tstm32_flash.ld
CFLAGS += -mlittle-endian -mthumb -mthumb-interwork -nostartfiles -mcpu=cortex-m4

ifeq ($(FLOAT_TYPE), hard)
CFLAGS += -fsingle-precision-constant -Wdouble-promotion
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard
else
CFLAGS += -msoft-float
endif

###################################################

vpath %.c src
vpath %.a lib

ROOT=$(shell pwd)

# Includes
CFLAGS += -I../../STLibrary/Core/cmsis -I../../STLibrary/Core/stm32
CFLAGS += -I../../STLibrary/Conf

# Library paths
LIBPATHS = -L../../STLibrary/StdPeriph -L../../STLibrary/USB_Device/Core
LIBPATHS += -L../../STLibrary/USB_Device/Class/cdc -L../../STLibrary/USB_OTG

# Libraries to link
LIBS = -lm -lstdperiph -lusbdevcore -lusbdevcdc -lusbcore

# Extra includes
CFLAGS += -I../../STLibrary/StdPeriph/inc
CFLAGS += -I../../STLibrary/USB_OTG/inc
CFLAGS += -I../../STLibrary/USB_Device/Core/inc
CFLAGS += -I../../STLibrary/USB_Device/Class/cdc/inc

# add startup file to build
SRCS += startup_stm32f4xx.S

OBJS = $(SRCS:.c=.o)

###################################################

.PHONY: lib

all: lib main.elf
	$(SIZE) main.elf

lib:
	$(MAKE) -C lib FLOAT_TYPE=$(FLOAT_TYPE) BINPATH=$(BINPATH)

main.elf: $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBPATHS) $(LIBS)
	$(OBJCOPY) -O ihex main.elf main.hex
	$(OBJCOPY) -O binary main.elf main.bin

clean:
	rm -f *.o *~
	rm -f main.elf
	rm -f main.hex
	rm -f main.bin

distclean: clean
	$(MAKE) clean -C lib
	rm -f *.log *.lst *.map *.s
