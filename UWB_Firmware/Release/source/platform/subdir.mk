################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/platform/deca_mutex.c \
../source/platform/deca_sleep.c \
../source/platform/deca_spi.c \
../source/platform/port.c \
../source/platform/stm32f0xx_it.c 

OBJS += \
./source/platform/deca_mutex.o \
./source/platform/deca_sleep.o \
./source/platform/deca_spi.o \
./source/platform/port.o \
./source/platform/stm32f0xx_it.o 

C_DEPS += \
./source/platform/deca_mutex.d \
./source/platform/deca_sleep.d \
./source/platform/deca_spi.d \
./source/platform/port.d \
./source/platform/stm32f0xx_it.d 


# Each subdirectory must supply rules for building sources it contributes
source/platform/%.o: ../source/platform/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -Wall -Wextra -DNDEBUG -DSTM32F051 -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -I"../header" -I"../system/header" -I"../system/header/cmsis" -I"../system/header/stm32f0-stdperiph" -I../header/platform -I../header/mLibs -I../header/decadriver -I../header/UAV -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


