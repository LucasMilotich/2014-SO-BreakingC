################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../comun/Structs.c \
../comun/UMV_Structs.c 

OBJS += \
./comun/Structs.o \
./comun/UMV_Structs.o 

C_DEPS += \
./comun/Structs.d \
./comun/UMV_Structs.d 


# Each subdirectory must supply rules for building sources it contributes
comun/%.o: ../comun/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


