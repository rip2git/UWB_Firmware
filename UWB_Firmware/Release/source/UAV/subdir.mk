################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/UAV/Ranging.c \
../source/UAV/Routing.c \
../source/UAV/SWM1000.c \
../source/UAV/TokenExt.c \
../source/UAV/Transceiver.c \
../source/UAV/USARTHandler.c \
../source/UAV/UserPack.c 

OBJS += \
./source/UAV/Ranging.o \
./source/UAV/Routing.o \
./source/UAV/SWM1000.o \
./source/UAV/TokenExt.o \
./source/UAV/Transceiver.o \
./source/UAV/USARTHandler.o \
./source/UAV/UserPack.o 

C_DEPS += \
./source/UAV/Ranging.d \
./source/UAV/Routing.d \
./source/UAV/SWM1000.d \
./source/UAV/TokenExt.d \
./source/UAV/Transceiver.d \
./source/UAV/USARTHandler.d \
./source/UAV/UserPack.d 


# Each subdirectory must supply rules for building sources it contributes
source/UAV/%.o: ../source/UAV/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -Wall -Wextra -fgnu89-inline -DNDEBUG -DSTM32F051 -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -I"../header" -I"../system/header" -I"../system/header/cmsis" -I"../system/header/stm32f0-stdperiph" -I../header/platform -I../header/mLibs -I../header/decadriver -I../header/UAV -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


