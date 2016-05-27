################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../sockets/Socket.c \
../sockets/Socket_Cliente.c \
../sockets/Socket_Servidor.c 

OBJS += \
./sockets/Socket.o \
./sockets/Socket_Cliente.o \
./sockets/Socket_Servidor.o 

C_DEPS += \
./sockets/Socket.d \
./sockets/Socket_Cliente.d \
./sockets/Socket_Servidor.d 


# Each subdirectory must supply rules for building sources it contributes
sockets/%.o: ../sockets/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


