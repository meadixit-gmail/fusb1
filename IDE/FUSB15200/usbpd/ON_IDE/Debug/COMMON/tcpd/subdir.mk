################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/CHOKRI/SRC/15200_4.0.0.01/External/common/tcpd/cable_svid_driv.c \
C:/CHOKRI/SRC/15200_4.0.0.01/External/common/tcpd/cable_tbt3_driv.c \
C:/CHOKRI/SRC/15200_4.0.0.01/External/common/tcpd/dp_driv.c \
C:/CHOKRI/SRC/15200_4.0.0.01/External/common/tcpd/observer.c \
C:/CHOKRI/SRC/15200_4.0.0.01/External/common/tcpd/policy.c \
C:/CHOKRI/SRC/15200_4.0.0.01/External/common/tcpd/protocol.c \
C:/CHOKRI/SRC/15200_4.0.0.01/External/common/tcpd/svid_driv.c \
C:/CHOKRI/SRC/15200_4.0.0.01/External/common/tcpd/typec.c \
C:/CHOKRI/SRC/15200_4.0.0.01/External/common/tcpd/vdm.c 

OBJS += \
./COMMON/tcpd/cable_svid_driv.o \
./COMMON/tcpd/cable_tbt3_driv.o \
./COMMON/tcpd/dp_driv.o \
./COMMON/tcpd/observer.o \
./COMMON/tcpd/policy.o \
./COMMON/tcpd/protocol.o \
./COMMON/tcpd/svid_driv.o \
./COMMON/tcpd/typec.o \
./COMMON/tcpd/vdm.o 

C_DEPS += \
./COMMON/tcpd/cable_svid_driv.d \
./COMMON/tcpd/cable_tbt3_driv.d \
./COMMON/tcpd/dp_driv.d \
./COMMON/tcpd/observer.d \
./COMMON/tcpd/policy.d \
./COMMON/tcpd/protocol.d \
./COMMON/tcpd/svid_driv.d \
./COMMON/tcpd/typec.d \
./COMMON/tcpd/vdm.d 


# Each subdirectory must supply rules for building sources it contributes
COMMON/tcpd/cable_svid_driv.o: C:/CHOKRI/SRC/15200_4.0.0.01/External/common/tcpd/cable_svid_driv.c COMMON/tcpd/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DCONFIG_SRC=1 -DCONFIG_VDM=1 -DCONFIG_EXTMSG=1 -DCONFIG_LOG=0 -DCONFIG_SLEEP=0 -DCONFIG_LEGACY_CHARGING=0 -DCONFIG_BC1P2_DCP=1 -DCONFIG_BC1P2_DCP_ADDIV=1 -DCONFIG_BC1P2_CSM=0 -DCONFIG_BC1P2_CDP=0 -DFUSB15200 -DHAL_USE_ASSERT -DCONFIG_DCDC=1 -DCONFIG_POWER_LIMITED=1 -DCONFIG_EPR=0 -DCONFIG_EPR_TEST=0 -DCONFIG_I2C_SLAVE=0 -DCONFIG_SNK=1 -DCONFIG_DRP=1 -DCONFIG_USB4=1 -DCONFIG_DP_ALT_MODE=0 -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/RTT" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Core/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Device/FUSB15200/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Device/FUSB15200" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/peripheral/fusb15xxx" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/tcpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/ps" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/bc12" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/dcdc" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/intf" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications/FUSB15200/usbpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

COMMON/tcpd/cable_tbt3_driv.o: C:/CHOKRI/SRC/15200_4.0.0.01/External/common/tcpd/cable_tbt3_driv.c COMMON/tcpd/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DCONFIG_SRC=1 -DCONFIG_VDM=1 -DCONFIG_EXTMSG=1 -DCONFIG_LOG=0 -DCONFIG_SLEEP=0 -DCONFIG_LEGACY_CHARGING=0 -DCONFIG_BC1P2_DCP=1 -DCONFIG_BC1P2_DCP_ADDIV=1 -DCONFIG_BC1P2_CSM=0 -DCONFIG_BC1P2_CDP=0 -DFUSB15200 -DHAL_USE_ASSERT -DCONFIG_DCDC=1 -DCONFIG_POWER_LIMITED=1 -DCONFIG_EPR=0 -DCONFIG_EPR_TEST=0 -DCONFIG_I2C_SLAVE=0 -DCONFIG_SNK=1 -DCONFIG_DRP=1 -DCONFIG_USB4=1 -DCONFIG_DP_ALT_MODE=0 -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/RTT" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Core/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Device/FUSB15200/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Device/FUSB15200" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/peripheral/fusb15xxx" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/tcpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/ps" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/bc12" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/dcdc" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/intf" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications/FUSB15200/usbpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

COMMON/tcpd/dp_driv.o: C:/CHOKRI/SRC/15200_4.0.0.01/External/common/tcpd/dp_driv.c COMMON/tcpd/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DCONFIG_SRC=1 -DCONFIG_VDM=1 -DCONFIG_EXTMSG=1 -DCONFIG_LOG=0 -DCONFIG_SLEEP=0 -DCONFIG_LEGACY_CHARGING=0 -DCONFIG_BC1P2_DCP=1 -DCONFIG_BC1P2_DCP_ADDIV=1 -DCONFIG_BC1P2_CSM=0 -DCONFIG_BC1P2_CDP=0 -DFUSB15200 -DHAL_USE_ASSERT -DCONFIG_DCDC=1 -DCONFIG_POWER_LIMITED=1 -DCONFIG_EPR=0 -DCONFIG_EPR_TEST=0 -DCONFIG_I2C_SLAVE=0 -DCONFIG_SNK=1 -DCONFIG_DRP=1 -DCONFIG_USB4=1 -DCONFIG_DP_ALT_MODE=0 -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/RTT" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Core/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Device/FUSB15200/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Device/FUSB15200" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/peripheral/fusb15xxx" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/tcpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/ps" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/bc12" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/dcdc" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/intf" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications/FUSB15200/usbpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

COMMON/tcpd/observer.o: C:/CHOKRI/SRC/15200_4.0.0.01/External/common/tcpd/observer.c COMMON/tcpd/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DCONFIG_SRC=1 -DCONFIG_VDM=1 -DCONFIG_EXTMSG=1 -DCONFIG_LOG=0 -DCONFIG_SLEEP=0 -DCONFIG_LEGACY_CHARGING=0 -DCONFIG_BC1P2_DCP=1 -DCONFIG_BC1P2_DCP_ADDIV=1 -DCONFIG_BC1P2_CSM=0 -DCONFIG_BC1P2_CDP=0 -DFUSB15200 -DHAL_USE_ASSERT -DCONFIG_DCDC=1 -DCONFIG_POWER_LIMITED=1 -DCONFIG_EPR=0 -DCONFIG_EPR_TEST=0 -DCONFIG_I2C_SLAVE=0 -DCONFIG_SNK=1 -DCONFIG_DRP=1 -DCONFIG_USB4=1 -DCONFIG_DP_ALT_MODE=0 -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/RTT" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Core/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Device/FUSB15200/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Device/FUSB15200" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/peripheral/fusb15xxx" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/tcpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/ps" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/bc12" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/dcdc" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/intf" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications/FUSB15200/usbpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

COMMON/tcpd/policy.o: C:/CHOKRI/SRC/15200_4.0.0.01/External/common/tcpd/policy.c COMMON/tcpd/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DCONFIG_SRC=1 -DCONFIG_VDM=1 -DCONFIG_EXTMSG=1 -DCONFIG_LOG=0 -DCONFIG_SLEEP=0 -DCONFIG_LEGACY_CHARGING=0 -DCONFIG_BC1P2_DCP=1 -DCONFIG_BC1P2_DCP_ADDIV=1 -DCONFIG_BC1P2_CSM=0 -DCONFIG_BC1P2_CDP=0 -DFUSB15200 -DHAL_USE_ASSERT -DCONFIG_DCDC=1 -DCONFIG_POWER_LIMITED=1 -DCONFIG_EPR=0 -DCONFIG_EPR_TEST=0 -DCONFIG_I2C_SLAVE=0 -DCONFIG_SNK=1 -DCONFIG_DRP=1 -DCONFIG_USB4=1 -DCONFIG_DP_ALT_MODE=0 -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/RTT" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Core/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Device/FUSB15200/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Device/FUSB15200" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/peripheral/fusb15xxx" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/tcpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/ps" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/bc12" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/dcdc" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/intf" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications/FUSB15200/usbpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

COMMON/tcpd/protocol.o: C:/CHOKRI/SRC/15200_4.0.0.01/External/common/tcpd/protocol.c COMMON/tcpd/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DCONFIG_SRC=1 -DCONFIG_VDM=1 -DCONFIG_EXTMSG=1 -DCONFIG_LOG=0 -DCONFIG_SLEEP=0 -DCONFIG_LEGACY_CHARGING=0 -DCONFIG_BC1P2_DCP=1 -DCONFIG_BC1P2_DCP_ADDIV=1 -DCONFIG_BC1P2_CSM=0 -DCONFIG_BC1P2_CDP=0 -DFUSB15200 -DHAL_USE_ASSERT -DCONFIG_DCDC=1 -DCONFIG_POWER_LIMITED=1 -DCONFIG_EPR=0 -DCONFIG_EPR_TEST=0 -DCONFIG_I2C_SLAVE=0 -DCONFIG_SNK=1 -DCONFIG_DRP=1 -DCONFIG_USB4=1 -DCONFIG_DP_ALT_MODE=0 -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/RTT" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Core/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Device/FUSB15200/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Device/FUSB15200" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/peripheral/fusb15xxx" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/tcpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/ps" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/bc12" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/dcdc" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/intf" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications/FUSB15200/usbpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

COMMON/tcpd/svid_driv.o: C:/CHOKRI/SRC/15200_4.0.0.01/External/common/tcpd/svid_driv.c COMMON/tcpd/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DCONFIG_SRC=1 -DCONFIG_VDM=1 -DCONFIG_EXTMSG=1 -DCONFIG_LOG=0 -DCONFIG_SLEEP=0 -DCONFIG_LEGACY_CHARGING=0 -DCONFIG_BC1P2_DCP=1 -DCONFIG_BC1P2_DCP_ADDIV=1 -DCONFIG_BC1P2_CSM=0 -DCONFIG_BC1P2_CDP=0 -DFUSB15200 -DHAL_USE_ASSERT -DCONFIG_DCDC=1 -DCONFIG_POWER_LIMITED=1 -DCONFIG_EPR=0 -DCONFIG_EPR_TEST=0 -DCONFIG_I2C_SLAVE=0 -DCONFIG_SNK=1 -DCONFIG_DRP=1 -DCONFIG_USB4=1 -DCONFIG_DP_ALT_MODE=0 -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/RTT" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Core/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Device/FUSB15200/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Device/FUSB15200" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/peripheral/fusb15xxx" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/tcpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/ps" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/bc12" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/dcdc" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/intf" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications/FUSB15200/usbpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

COMMON/tcpd/typec.o: C:/CHOKRI/SRC/15200_4.0.0.01/External/common/tcpd/typec.c COMMON/tcpd/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DCONFIG_SRC=1 -DCONFIG_VDM=1 -DCONFIG_EXTMSG=1 -DCONFIG_LOG=0 -DCONFIG_SLEEP=0 -DCONFIG_LEGACY_CHARGING=0 -DCONFIG_BC1P2_DCP=1 -DCONFIG_BC1P2_DCP_ADDIV=1 -DCONFIG_BC1P2_CSM=0 -DCONFIG_BC1P2_CDP=0 -DFUSB15200 -DHAL_USE_ASSERT -DCONFIG_DCDC=1 -DCONFIG_POWER_LIMITED=1 -DCONFIG_EPR=0 -DCONFIG_EPR_TEST=0 -DCONFIG_I2C_SLAVE=0 -DCONFIG_SNK=1 -DCONFIG_DRP=1 -DCONFIG_USB4=1 -DCONFIG_DP_ALT_MODE=0 -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/RTT" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Core/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Device/FUSB15200/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Device/FUSB15200" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/peripheral/fusb15xxx" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/tcpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/ps" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/bc12" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/dcdc" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/intf" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications/FUSB15200/usbpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

COMMON/tcpd/vdm.o: C:/CHOKRI/SRC/15200_4.0.0.01/External/common/tcpd/vdm.c COMMON/tcpd/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DCONFIG_SRC=1 -DCONFIG_VDM=1 -DCONFIG_EXTMSG=1 -DCONFIG_LOG=0 -DCONFIG_SLEEP=0 -DCONFIG_LEGACY_CHARGING=0 -DCONFIG_BC1P2_DCP=1 -DCONFIG_BC1P2_DCP_ADDIV=1 -DCONFIG_BC1P2_CSM=0 -DCONFIG_BC1P2_CDP=0 -DFUSB15200 -DHAL_USE_ASSERT -DCONFIG_DCDC=1 -DCONFIG_POWER_LIMITED=1 -DCONFIG_EPR=0 -DCONFIG_EPR_TEST=0 -DCONFIG_I2C_SLAVE=0 -DCONFIG_SNK=1 -DCONFIG_DRP=1 -DCONFIG_USB4=1 -DCONFIG_DP_ALT_MODE=0 -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/RTT" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Core/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Device/FUSB15200/Include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Device/FUSB15200" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/peripheral/fusb15xxx" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Drivers/" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/include" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/tcpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/ps" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/bc12" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/dcdc" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../External/common/intf" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications/FUSB15200/usbpd" -I"C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../Applications" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


