################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include RTT/subdir.mk
-include Drivers/Device/15xxx/subdir.mk
-include Drivers/subdir.mk
-include Device/subdir.mk
-include COMMON/util/subdir.mk
-include COMMON/tcpd/subdir.mk
-include COMMON/ps/subdir.mk
-include COMMON/intf/subdir.mk
-include COMMON/dcdc/subdir.mk
-include COMMON/bc1p2/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := FUSB15200_USBPD
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
FUSB15200_USBPD.bin \

SECONDARY_LIST += \
FUSB15200_USBPD.lst \

SECONDARY_SIZE += \
FUSB15200_USBPD.siz \


# All Target
all: main-build

# Main-build Target
main-build: FUSB15200_USBPD.elf secondary-outputs

# Tool invocations
FUSB15200_USBPD.elf: $(OBJS) $(USER_OBJS) makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM Cross C Linker'
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -T "C:/CHOKRI/SRC/15200_4.0.0.01/IDE/FUSB15200/usbpd/ON_IDE/../../../../CMSIS/Device/FUSB15200/Source/GCC/gcc_arm.ld" -nostartfiles -Xlinker --gc-sections -Wl,-Map,"FUSB15200_USBPD.map" --specs=nano.specs -o "FUSB15200_USBPD.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

FUSB15200_USBPD.bin: FUSB15200_USBPD.elf makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo 'Invoking: GNU ARM Cross Create Flash Image'
	arm-none-eabi-objcopy -O binary "FUSB15200_USBPD.elf"  "FUSB15200_USBPD.bin"
	@echo 'Finished building: $@'
	@echo ' '

FUSB15200_USBPD.lst: FUSB15200_USBPD.elf makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo 'Invoking: GNU ARM Cross Create Listing'
	arm-none-eabi-objdump --source --all-headers --demangle --line-numbers --wide "FUSB15200_USBPD.elf" > "FUSB15200_USBPD.lst"
	@echo 'Finished building: $@'
	@echo ' '

FUSB15200_USBPD.siz: FUSB15200_USBPD.elf makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo 'Invoking: GNU ARM Cross Print Size'
	arm-none-eabi-size --format=berkeley "FUSB15200_USBPD.elf"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(SECONDARY_FLASH)$(SECONDARY_LIST)$(SECONDARY_SIZE)$(ASM_DEPS)$(S_UPPER_DEPS)$(C_DEPS) FUSB15200_USBPD.elf
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST) $(SECONDARY_SIZE)

.PHONY: all clean dependents main-build

-include ../makefile.targets
