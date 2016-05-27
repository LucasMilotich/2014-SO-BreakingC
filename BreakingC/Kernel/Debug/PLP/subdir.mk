################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../PLP/SocketServerPLP.c \
../PLP/funcionesPLP.c \
../PLP/plp.c 

OBJS += \
./PLP/SocketServerPLP.o \
./PLP/funcionesPLP.o \
./PLP/plp.o 

C_DEPS += \
./PLP/SocketServerPLP.d \
./PLP/funcionesPLP.d \
./PLP/plp.d 


# Each subdirectory must supply rules for building sources it contributes
PLP/%.o: ../PLP/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2014-1c-breaking-c/BreakingC/Libraries" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


