-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include src/WinSubdir.mk
-include src/Utils/WinSubdir.mk
-include src/TestsRunner/WinSubdir.mk
-include src/RandomInputGenerator/WinSubdir.mk
-include src/KernelExecutor/WinSubdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: tests

# Setting environment
env:
	@echo 'Setting environment varaibles'
	setenv.sh
#	@set Framework35Version=v3.5
#	@set PATH="C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\bin\amd64":$PATH
#	@export Framework35Version
#	@echo $$Framework35Version

# Tool invocations
tests: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Visual C++ Linker'
	link  /out:"tests.exe" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(C++_DEPS)$(OBJS)$(C_DEPS)$(CC_DEPS)$(CPP_DEPS)$(EXECUTABLES)$(CXX_DEPS)$(C_UPPER_DEPS) tests
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
