################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/adc.c \
../src/controller.c \
../src/dht11.c \
../src/display.c \
../src/display_highlevel.c \
../src/downconv_data.c \
../src/fan_pump.c \
../src/font8x8.c \
../src/gpio.c \
../src/main.c \
../src/syscalls.c \
../src/system_stm32f4xx.c \
../src/timer.c \
../src/uart.c \
../src/upconv_data.c 

OBJS += \
./src/adc.o \
./src/controller.o \
./src/dht11.o \
./src/display.o \
./src/display_highlevel.o \
./src/downconv_data.o \
./src/fan_pump.o \
./src/font8x8.o \
./src/gpio.o \
./src/main.o \
./src/syscalls.o \
./src/system_stm32f4xx.o \
./src/timer.o \
./src/uart.o \
./src/upconv_data.o 

C_DEPS += \
./src/adc.d \
./src/controller.d \
./src/dht11.d \
./src/display.d \
./src/display_highlevel.d \
./src/downconv_data.d \
./src/fan_pump.d \
./src/font8x8.d \
./src/gpio.d \
./src/main.d \
./src/syscalls.d \
./src/system_stm32f4xx.d \
./src/timer.d \
./src/uart.d \
./src/upconv_data.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -DSTM32 -DSTM32F4 -DSTM32F446RETx -DSTM32F446xx -DUSE_STDPERIPH_DRIVER -I"/Daten1/Daten/funk/sat/Amsat-Boards/V3_4/PA250controller/pa250ctrl/StdPeriph_Driver/inc" -I"/Daten1/Daten/funk/sat/Amsat-Boards/V3_4/PA250controller/pa250ctrl/inc" -I"/Daten1/Daten/funk/sat/Amsat-Boards/V3_4/PA250controller/pa250ctrl/CMSIS/device" -I"/Daten1/Daten/funk/sat/Amsat-Boards/V3_4/PA250controller/pa250ctrl/CMSIS/core" -O3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


