################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../PCP/PCP.c \
../PCP/funcionesAdministrativas.c \
../PCP/operacionesPCP.c 

OBJS += \
./PCP/PCP.o \
./PCP/funcionesAdministrativas.o \
./PCP/operacionesPCP.o 

C_DEPS += \
./PCP/PCP.d \
./PCP/funcionesAdministrativas.d \
./PCP/operacionesPCP.d 


# Each subdirectory must supply rules for building sources it contributes
PCP/%.o: ../PCP/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2014-1c-breaking-c/BreakingC/Libraries" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


