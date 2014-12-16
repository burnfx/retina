################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/ParameterManager.cpp \
../src/RetinaManager.cpp \
../src/client.cpp \
../src/eDVSGL.cpp \
../src/main.cpp \
../src/server.cpp 

OBJS += \
./src/ParameterManager.o \
./src/RetinaManager.o \
./src/client.o \
./src/eDVSGL.o \
./src/main.o \
./src/server.o 

CPP_DEPS += \
./src/ParameterManager.d \
./src/RetinaManager.d \
./src/client.d \
./src/eDVSGL.d \
./src/main.d \
./src/server.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=gnu++0x -I"/home/richi-ubuntu/workspace/retina/LibOVR/Include" -I"/home/richi-ubuntu/workspace/retina/src/shader" -I"/home/richi-ubuntu/workspace/retina/src/edvs" -I"/home/richi-ubuntu/workspace/retina/LibOVR/Src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


