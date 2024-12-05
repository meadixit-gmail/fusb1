################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/CHOKRI/SRC/15200_4.0.0.01/External/common/intf/dev_tcpd.c \
C:/CHOKRI/SRC/15200_4.0.0.01/External/common/intf/port.c \
C:/CHOKRI/SRC/15200_4.0.0.01/External/common/intf/port_bc1p2.c \
C:/CHOKRI/SRC/15200_4.0.0.01/External/common/intf/port_dcdc.c \
C:/CHOKRI/SRC/15200_4.0.0.01/External/common/intf/port_ps.c \
C:/CHOKRI/SRC/15200_4.0.0.01/External/common/intf/port_tcpd.c 

OBJS += \
./COMMON/intf/dev_tcpd.o \
./COMMON/intf/port.o \
./COMMON/intf/port_bc1p2.o \
./COMMON/intf/port_dcdc.o \
./COMMON/intf/port_ps.o \
./COMMON/intf/port_tcpd.o 

C_DEPS += \
./COMMON/intf/dev_tcpd.d \
./COMMON/intf/port.d \
./COMMON/intf/port_bc1p2.d \
./COMMON/intf/port_dcdc.d \
./COMMON/intf/port_ps.d \
./COMMON/intf/port_tcpd.d 


# Each subdirectory must supply rules for building sources it contributes
COMMON/intf/dev_tcpd.o: C:/CHOKRI/SRC/15200_4.0.0.01/External/common/intf/dev_tcpd.c COMMON/intf/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DCONFIG_SRC=1 -DCONFIG_VDM=1 -DCONFIG_EXTMSG=1 -DCONFIG_LOG=0 -DCONFIG_SLEEP=0 -DCONFIG_LEGACY_CHARGING=0 -DCONFIG_BC1P2_DCP=1 -DCONFIG_BC1P2_DCP_ADDIV=1 -DCONFIG_BC1P2_CSM=0 -DCONFIG_BC1P2_CDP=0 -DFUSB15200 -DHAL_USE_ASSERT -DCONFIG_DCDC=1 -DCONFIG_POWER_LIMITED=1 -DCONFIG_EPR=0 -DCONFIG_EPR_TEST=0 -DCONFIG_I2C_SLAVE=0 -DCONFIG_SNK=1 -DCONFIG_DRP=1 -DCONFIG_USB4=1 -DCONFIG_DP_ALT_MODE=0 -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/RTT" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Core/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Device/FUSB15200/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Device/FUSB15200" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/peripheral/fusb15xxx" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/tcpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/ps" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/bc12" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/dcdc" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/intf" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications/FUSB15200/usbpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

COMMON/intf/port.o: C:/CHOKRI/SRC/15200_4.0.0.01/External/common/intf/port.c COMMON/intf/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DCONFIG_SRC=1 -DCONFIG_VDM=1 -DCONFIG_EXTMSG=1 -DCONFIG_LOG=0 -DCONFIG_SLEEP=0 -DCONFIG_LEGACY_CHARGING=0 -DCONFIG_BC1P2_DCP=1 -DCONFIG_BC1P2_DCP_ADDIV=1 -DCONFIG_BC1P2_CSM=0 -DCONFIG_BC1P2_CDP=0 -DFUSB15200 -DHAL_USE_ASSERT -DCONFIG_DCDC=1 -DCONFIG_POWER_LIMITED=1 -DCONFIG_EPR=0 -DCONFIG_EPR_TEST=0 -DCONFIG_I2C_SLAVE=0 -DCONFIG_SNK=1 -DCONFIG_DRP=1 -DCONFIG_USB4=1 -DCONFIG_DP_ALT_MODE=0 -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/RTT" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Core/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Device/FUSB15200/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Device/FUSB15200" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/peripheral/fusb15xxx" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/tcpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/ps" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/bc12" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/dcdc" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/intf" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications/FUSB15200/usbpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

COMMON/intf/port_bc1p2.o: C:/CHOKRI/SRC/15200_4.0.0.01/External/common/intf/port_bc1p2.c COMMON/intf/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DCONFIG_SRC=1 -DCONFIG_VDM=1 -DCONFIG_EXTMSG=1 -DCONFIG_LOG=0 -DCONFIG_SLEEP=0 -DCONFIG_LEGACY_CHARGING=0 -DCONFIG_BC1P2_DCP=1 -DCONFIG_BC1P2_DCP_ADDIV=1 -DCONFIG_BC1P2_CSM=0 -DCONFIG_BC1P2_CDP=0 -DFUSB15200 -DHAL_USE_ASSERT -DCONFIG_DCDC=1 -DCONFIG_POWER_LIMITED=1 -DCONFIG_EPR=0 -DCONFIG_EPR_TEST=0 -DCONFIG_I2C_SLAVE=0 -DCONFIG_SNK=1 -DCONFIG_DRP=1 -DCONFIG_USB4=1 -DCONFIG_DP_ALT_MODE=0 -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/RTT" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Core/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Device/FUSB15200/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Device/FUSB15200" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/peripheral/fusb15xxx" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/tcpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/ps" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/bc12" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/dcdc" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/intf" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications/FUSB15200/usbpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

COMMON/intf/port_dcdc.o: C:/CHOKRI/SRC/15200_4.0.0.01/External/common/intf/port_dcdc.c COMMON/intf/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DCONFIG_SRC=1 -DCONFIG_VDM=1 -DCONFIG_EXTMSG=1 -DCONFIG_LOG=0 -DCONFIG_SLEEP=0 -DCONFIG_LEGACY_CHARGING=0 -DCONFIG_BC1P2_DCP=1 -DCONFIG_BC1P2_DCP_ADDIV=1 -DCONFIG_BC1P2_CSM=0 -DCONFIG_BC1P2_CDP=0 -DFUSB15200 -DHAL_USE_ASSERT -DCONFIG_DCDC=1 -DCONFIG_POWER_LIMITED=1 -DCONFIG_EPR=0 -DCONFIG_EPR_TEST=0 -DCONFIG_I2C_SLAVE=0 -DCONFIG_SNK=1 -DCONFIG_DRP=1 -DCONFIG_USB4=1 -DCONFIG_DP_ALT_MODE=0 -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/RTT" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Core/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Device/FUSB15200/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Device/FUSB15200" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/peripheral/fusb15xxx" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/tcpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/ps" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/bc12" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/dcdc" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/intf" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications/FUSB15200/usbpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

COMMON/intf/port_ps.o: C:/CHOKRI/SRC/15200_4.0.0.01/External/common/intf/port_ps.c COMMON/intf/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DCONFIG_SRC=1 -DCONFIG_VDM=1 -DCONFIG_EXTMSG=1 -DCONFIG_LOG=0 -DCONFIG_SLEEP=0 -DCONFIG_LEGACY_CHARGING=0 -DCONFIG_BC1P2_DCP=1 -DCONFIG_BC1P2_DCP_ADDIV=1 -DCONFIG_BC1P2_CSM=0 -DCONFIG_BC1P2_CDP=0 -DFUSB15200 -DHAL_USE_ASSERT -DCONFIG_DCDC=1 -DCONFIG_POWER_LIMITED=1 -DCONFIG_EPR=0 -DCONFIG_EPR_TEST=0 -DCONFIG_I2C_SLAVE=0 -DCONFIG_SNK=1 -DCONFIG_DRP=1 -DCONFIG_USB4=1 -DCONFIG_DP_ALT_MODE=0 -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/RTT" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Core/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Device/FUSB15200/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Device/FUSB15200" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/peripheral/fusb15xxx" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/tcpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/ps" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/bc12" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/dcdc" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/intf" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications/FUSB15200/usbpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

COMMON/intf/port_tcpd.o: C:/CHOKRI/SRC/15200_4.0.0.01/External/common/intf/port_tcpd.c COMMON/intf/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DCONFIG_SRC=1 -DCONFIG_VDM=1 -DCONFIG_EXTMSG=1 -DCONFIG_LOG=0 -DCONFIG_SLEEP=0 -DCONFIG_LEGACY_CHARGING=0 -DCONFIG_BC1P2_DCP=1 -DCONFIG_BC1P2_DCP_ADDIV=1 -DCONFIG_BC1P2_CSM=0 -DCONFIG_BC1P2_CDP=0 -DFUSB15200 -DHAL_USE_ASSERT -DCONFIG_DCDC=1 -DCONFIG_POWER_LIMITED=1 -DCONFIG_EPR=0 -DCONFIG_EPR_TEST=0 -DCONFIG_I2C_SLAVE=0 -DCONFIG_SNK=1 -DCONFIG_DRP=1 -DCONFIG_USB4=1 -DCONFIG_DP_ALT_MODE=0 -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/RTT" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Core/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Device/FUSB15200/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Device/FUSB15200" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/peripheral/fusb15xxx" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/tcpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/ps" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/bc12" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/dcdc" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/intf" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications/FUSB15200/usbpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


