CMSIS_PATH = $(PROJDIR)/CMSIS
LDSCRIPT := $(CMSIS_PATH)/Device/$(DEVICE)/Source/GCC/gcc_arm.ld
INCDIR += -I$(CMSIS_PATH)/Core/Include\
         -I$(CMSIS_PATH)/Device/$(DEVICE)/Include
CSRC += $(CMSIS_PATH)/Device/$(DEVICE)/Source/$(COMPILER)/startup_$(DEVICE).c \
        $(CMSIS_PATH)/Device/$(DEVICE)/Source/system_$(DEVICE).c

