
CFLAGS += \
	-I./inc \
	-I./src \
	-I./src/dbg \
	-I./lib/ \
	
CFLAGS += \
	-DDEBUG \
	-DSTM32F10X_MD_VL \
	-DHSE_VALUE=8000000 \
	-DUSE_STDPERIPH_DRIVER \
	-DUSE_FULL_ASSERT \
	\
	-I./sdk/include/ \
	-I./sdk/include/arm \
	-I./sdk/include/cmsis/ \
	-I./sdk/include/cortexm/ \
	-I./sdk/include/diag \
	-I./sdk/include/stm32f1-stdperiph \
	
export SRC := \
	$(wildcard ./src/*.c*) \
	$(wildcard ./src/dbg/*.c*) \
	$(wildcard ./lib/src/*.c) \
	\
	./sdk/src/stm32f1-stdperiph/stm32f10x_rcc.c \
	./sdk/src/stm32f1-stdperiph/stm32f10x_gpio.c \
	./sdk/src/stm32f1-stdperiph/stm32f10x_adc.c \
	./sdk/src/stm32f1-stdperiph/stm32f10x_iwdg.c \
	./sdk/src/stm32f1-stdperiph/misc.c \
	./sdk/src/stm32f1-stdperiph/stm32f10x_tim.c \
	./sdk/src/stm32f1-stdperiph/stm32f10x_usart.c \
	./sdk/src/stm32f1-stdperiph/stm32f10x_dma.c \
	./sdk/src/stm32f1-stdperiph/stm32f10x_i2c.c \

