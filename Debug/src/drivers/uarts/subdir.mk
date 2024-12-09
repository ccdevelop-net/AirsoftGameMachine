################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/drivers/uarts/ms-timer.cpp 

CPP_DEPS += \
./src/drivers/uarts/ms-timer.d 

OBJS += \
./src/drivers/uarts/ms-timer.o 


# Each subdirectory must supply rules for building sources it contributes
src/drivers/uarts/%.o: ../src/drivers/uarts/%.cpp src/drivers/uarts/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -std=c++17 -I"/home/liquidsnake/work/projects/airsoft-bot/control_unit/software/Airsoft/include" -O0 -g3 -Wall -c -fmessage-length=0 -pthread -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-drivers-2f-uarts

clean-src-2f-drivers-2f-uarts:
	-$(RM) ./src/drivers/uarts/ms-timer.d ./src/drivers/uarts/ms-timer.o

.PHONY: clean-src-2f-drivers-2f-uarts

