SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_PROCESSOR cortex-m0plus)

SET(LD_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/lds/gcc_arm.ld")
SET(MCU_FLAGS "-mthumb -mcpu=${CMAKE_SYSTEM_PROCESSOR} -DFUSB15200 -ffreestanding")
SET(LD_FLAGS "-T${LD_SCRIPT} -nostartfiles --specs=nosys.specs --specs=nano.specs")

include("${CMAKE_CURRENT_LIST_DIR}/common/arm-none-eabi.cmake")
