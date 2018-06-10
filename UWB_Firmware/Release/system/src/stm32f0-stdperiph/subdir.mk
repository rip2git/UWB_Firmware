################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../system/src/stm32f0-stdperiph/stm32f0xx_adc.c \
../system/src/stm32f0-stdperiph/stm32f0xx_dma.c \
../system/src/stm32f0-stdperiph/stm32f0xx_exti.c \
../system/src/stm32f0-stdperiph/stm32f0xx_gpio.c \
../system/src/stm32f0-stdperiph/stm32f0xx_iwdg.c \
../system/src/stm32f0-stdperiph/stm32f0xx_misc.c \
../system/src/stm32f0-stdperiph/stm32f0xx_rcc.c \
../system/src/stm32f0-stdperiph/stm32f0xx_spi.c \
../system/src/stm32f0-stdperiph/stm32f0xx_syscfg.c \
../system/src/stm32f0-stdperiph/stm32f0xx_tim.c \
../system/src/stm32f0-stdperiph/stm32f0xx_usart.c 

OBJS += \
./system/src/stm32f0-stdperiph/stm32f0xx_adc.o \
./system/src/stm32f0-stdperiph/stm32f0xx_dma.o \
./system/src/stm32f0-stdperiph/stm32f0xx_exti.o \
./system/src/stm32f0-stdperiph/stm32f0xx_gpio.o \
./system/src/stm32f0-stdperiph/stm32f0xx_iwdg.o \
./system/src/stm32f0-stdperiph/stm32f0xx_misc.o \
./system/src/stm32f0-stdperiph/stm32f0xx_rcc.o \
./system/src/stm32f0-stdperiph/stm32f0xx_spi.o \
./system/src/stm32f0-stdperiph/stm32f0xx_syscfg.o \
./system/src/stm32f0-stdperiph/stm32f0xx_tim.o \
./system/src/stm32f0-stdperiph/stm32f0xx_usart.o 

C_DEPS += \
./system/src/stm32f0-stdperiph/stm32f0xx_adc.d \
./system/src/stm32f0-stdperiph/stm32f0xx_dma.d \
./system/src/stm32f0-stdperiph/stm32f0xx_exti.d \
./system/src/stm32f0-stdperiph/stm32f0xx_gpio.d \
./system/src/stm32f0-stdperiph/stm32f0xx_iwdg.d \
./system/src/stm32f0-stdperiph/stm32f0xx_misc.d \
./system/src/stm32f0-stdperiph/stm32f0xx_rcc.d \
./system/src/stm32f0-stdperiph/stm32f0xx_spi.d \
./system/src/stm32f0-stdperiph/stm32f0xx_syscfg.d \
./system/src/stm32f0-stdperiph/stm32f0xx_tim.d \
./system/src/stm32f0-stdperiph/stm32f0xx_usart.d 


# Each subdirectory must supply rules for building sources it contributes
system/src/stm32f0-stdperiph/%.o: ../system/src/stm32f0-stdperiph/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -Wall -Wextra -fgnu89-inline -DNDEBUG -DSTM32F051 -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -I"../header" -I"../system/header" -I"../system/header/cmsis" -I"../system/header/stm32f0-stdperiph" -I../header/platform -I../header/mLibs -I../header/decadriver -I../header/UAV -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


