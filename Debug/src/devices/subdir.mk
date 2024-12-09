################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/devices/ebytelorae220.cpp \
../src/devices/i2c-display.cpp \
../src/devices/i2ckeypad.cpp \
../src/devices/pcf8574.cpp 

CPP_DEPS += \
./src/devices/ebytelorae220.d \
./src/devices/i2c-display.d \
./src/devices/i2ckeypad.d \
./src/devices/pcf8574.d 

OBJS += \
./src/devices/ebytelorae220.o \
./src/devices/i2c-display.o \
./src/devices/i2ckeypad.o \
./src/devices/pcf8574.o 


# Each subdirectory must supply rules for building sources it contributes
src/devices/%.o: ../src/devices/%.cpp src/devices/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -std=c++17 -I"/home/liquidsnake/work/projects/airsoft-bot/control_unit/software/Airsoft/include" -O0 -g3 -Wall -c -fmessage-length=0 -pthread -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-devices

clean-src-2f-devices:
	-$(RM) ./src/devices/ebytelorae220.d ./src/devices/ebytelorae220.o ./src/devices/i2c-display.d ./src/devices/i2c-display.o ./src/devices/i2ckeypad.d ./src/devices/i2ckeypad.o ./src/devices/pcf8574.d ./src/devices/pcf8574.o

.PHONY: clean-src-2f-devices

