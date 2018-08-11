################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/mLibs/CheckSum.c \
../source/mLibs/EventTimer.c \
../source/mLibs/Random.c \
../source/mLibs/SystemTimer.c \
../source/mLibs/mUSART_DMA.c 

OBJS += \
./source/mLibs/CheckSum.o \
./source/mLibs/EventTimer.o \
./source/mLibs/Random.o \
./source/mLibs/SystemTimer.o \
./source/mLibs/mUSART_DMA.o 

C_DEPS += \
./source/mLibs/CheckSum.d \
./source/mLibs/EventTimer.d \
./source/mLibs/Random.d \
./source/mLibs/SystemTimer.d \
./source/mLibs/mUSART_DMA.d 


# Each subdirectory must supply rules for building sources it contributes
source/mLibs/%.o: ../source/mLibs/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -Wall -Wextra -fgnu89-inline -DNDEBUG -DSTM32F051 -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -I"../header" -I"../system/header" -I"../system/header/cmsis" -I"../system/header/stm32f0-stdperiph" -I../header/platform -I../header/mLibs -I../header/decadriver -I../header/UAV -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


