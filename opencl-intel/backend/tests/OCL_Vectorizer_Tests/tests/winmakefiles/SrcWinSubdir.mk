################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/main.cpp 

OBJS += \
./src/main.obj 

CPP_DEPS += \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.obj: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Visual C++ Compiler'
	cl /EHsc /MT /c -oF"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


