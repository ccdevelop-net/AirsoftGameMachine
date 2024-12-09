################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/classes/print.cpp 

CPP_DEPS += \
./src/classes/print.d 

OBJS += \
./src/classes/print.o 


# Each subdirectory must supply rules for building sources it contributes
src/classes/%.o: ../src/classes/%.cpp src/classes/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -std=c++17 -I"/home/liquidsnake/work/projects/airsoft-bot/control_unit/software/Airsoft/include" -O0 -g3 -Wall -c -fmessage-length=0 -pthread -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-classes

clean-src-2f-classes:
	-$(RM) ./src/classes/print.d ./src/classes/print.o

.PHONY: clean-src-2f-classes

