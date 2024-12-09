################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/airsoftmanager.cpp \
../src/gps.cpp \
../src/inout.cpp \
../src/main.cpp \
../src/utility.cpp \
../src/wireless.cpp 

CPP_DEPS += \
./src/airsoftmanager.d \
./src/gps.d \
./src/inout.d \
./src/main.d \
./src/utility.d \
./src/wireless.d 

OBJS += \
./src/airsoftmanager.o \
./src/gps.o \
./src/inout.o \
./src/main.o \
./src/utility.o \
./src/wireless.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -std=c++17 -I"/home/liquidsnake/work/projects/airsoft-bot/control_unit/software/Airsoft/include" -O0 -g3 -Wall -c -fmessage-length=0 -pthread -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/airsoftmanager.d ./src/airsoftmanager.o ./src/gps.d ./src/gps.o ./src/inout.d ./src/inout.o ./src/main.d ./src/main.o ./src/utility.d ./src/utility.o ./src/wireless.d ./src/wireless.o

.PHONY: clean-src

