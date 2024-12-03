################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/drivers/gpio.cpp \
../src/drivers/i2c.cpp \
../src/drivers/uarts.cpp 

CPP_DEPS += \
./src/drivers/gpio.d \
./src/drivers/i2c.d \
./src/drivers/uarts.d 

OBJS += \
./src/drivers/gpio.o \
./src/drivers/i2c.o \
./src/drivers/uarts.o 


# Each subdirectory must supply rules for building sources it contributes
src/drivers/%.o: ../src/drivers/%.cpp src/drivers/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -std=c++17 -I"/home/liquidsnake/work/projects/airsoft-bot/control_unit/software/Airsoft/include" -O0 -g3 -Wall -c -fmessage-length=0 -pthread -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-drivers

clean-src-2f-drivers:
	-$(RM) ./src/drivers/gpio.d ./src/drivers/gpio.o ./src/drivers/i2c.d ./src/drivers/i2c.o ./src/drivers/uarts.d ./src/drivers/uarts.o

.PHONY: clean-src-2f-drivers

