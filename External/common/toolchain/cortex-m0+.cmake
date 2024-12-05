SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_PROCESSOR cortex-m0plus)

SET(MCU_FLAGS "-mthumb -mcpu=${CMAKE_SYSTEM_PROCESSOR} -DARM_CM0PLUS")
SET(LD_FLAGS "-nostartfiles --specs=nosys.specs")

include("${CMAKE_CURRENT_LIST_DIR}/common/arm-none-eabi.cmake")
