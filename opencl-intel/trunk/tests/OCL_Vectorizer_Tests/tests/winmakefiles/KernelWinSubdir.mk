################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/KernelExecutor/ArrayParameter.cpp \
../src/KernelExecutor/Execution.cpp \
../src/KernelExecutor/KerenelExecutor.cpp \
../src/KernelExecutor/RegularParameter.cpp 

OBJS += \
./src/KernelExecutor/ArrayParameter.obj \
./src/KernelExecutor/Execution.obj \
./src/KernelExecutor/KerenelExecutor.obj \
./src/KernelExecutor/RegularParameter.obj 

CPP_DEPS += \
./src/KernelExecutor/ArrayParameter.d \
./src/KernelExecutor/Execution.d \
./src/KernelExecutor/KerenelExecutor.d \
./src/KernelExecutor/RegularParameter.d 


# Each subdirectory must supply rules for building sources it contributes
src/KernelExecutor/%.obj: ../src/KernelExecutor/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Visual C++ Compiler'
	cl /EHsc /MT /c -oF"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


