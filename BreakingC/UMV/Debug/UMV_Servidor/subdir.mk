################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../UMV_Servidor/UMV_Servidor.c 

OBJS += \
./UMV_Servidor/UMV_Servidor.o 

C_DEPS += \
./UMV_Servidor/UMV_Servidor.d 


# Each subdirectory must supply rules for building sources it contributes
UMV_Servidor/%.o: ../UMV_Servidor/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2014-1c-breaking-c/BreakingC/Libraries" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


