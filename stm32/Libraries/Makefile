CC=arm-none-eabi-gcc
AR=arm-none-eabi-ar

###########################################

vpath %.c STM32F30x_StdPeriph_Driver/src

CFLAGS  = -g -O2 -Wall
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -march=armv7e-m
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CFLAGS += -ffreestanding -nostdlib
CFLAGS += -includestm32f30x_conf.h 
CFLAGS += -ICMSIS/Include 
CFLAGS += -ICMSIS/Device/ST/STM32F30x/Include 
CFLAGS += -ISTM32F30x_StdPeriph_Driver/inc
CFLAGS += -I../inc

SRCS = stm32f30x_adc.c stm32f30x_can.c stm32f30x_comp.c stm32f30x_crc.c \
       stm32f30x_dac.c stm32f30x_dbgmcu.c stm32f30x_dma.c \
       stm32f30x_exti.c stm32f30x_flash.c stm32f30x_gpio.c \
       stm32f30x_i2c.c stm32f30x_iwdg.c stm32f30x_misc.c stm32f30x_opamp.c \
       stm32f30x_pwr.c stm32f30x_rcc.c stm32f30x_rtc.c stm32f30x_spi.c \
       stm32f30x_syscfg.c stm32f30x_tim.c stm32f30x_usart.c \
       stm32f30x_wwdg.c 

OBJS = $(addprefix objs/,$(SRCS:.c=.o))
DEPS = $(addprefix deps/,$(SRCS:.c=.d))

.PHONY: all clean

all: libstm32f3.a

-include $(DEPS)

dirs:
	mkdir -p deps objs
	touch dirs

objs/%.o : %.c dirs
	$(CC) $(CFLAGS) -c -o $@ $< -MMD -MF deps/$(*F).d

libstm32f3.a: $(OBJS)
	$(AR) -r $@ $(OBJS)

clean:
	rm -f dirs $(OBJS) $(DEPS) libstm32f3.a

