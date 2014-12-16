################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/shader/shader.cpp 

OBJS += \
./src/shader/shader.o 

CPP_DEPS += \
./src/shader/shader.d 


# Each subdirectory must supply rules for building sources it contributes
src/shader/%.o: ../src/shader/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=gnu++0x -I"/home/user/workspace2/retina/LibOVR/Include" -I"/home/user/workspace2/retina/src/shader" -I"/home/user/workspace2/retina/src/edvs" -I"/home/user/workspace2/retina/LibOVR/Src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


