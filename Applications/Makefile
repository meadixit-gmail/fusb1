# Define Project Variablses
PROJECT := Application_FUSB15200_USBPD
DEVICE := FUSB15200
PROJDIR := $(abspath ../../..)
DOCDIR = $(PROJDIR)/Utilities/doxygen
COMPILER = GCC
# define toolchains, build tools
PREFIX = arm-none-eabi-
CC  = $(PREFIX)gcc
CPP = $(PREFIX)g++
LD  = $(PREFIX)gcc
AS  = $(PREFIX)gcc -x assembler-with-cpp
AR  = $(PREFIX)ar
OBJSIZE = $(PREFIX)size
OBJDUMP = $(PREFIX)objdump
OBJCOPY = $(PREFIX)objcopy

# MPU Architecture
# Optimize level -O0, O1, -O2, -Os etc
MCU = cortex-m0plus
OPTIMIZE = -O0

# Output directories
BUILDDIR := Build
OBJDIR := $(BUILDDIR)/obj
TESTDIR := Outputs
CPPCHECKDIR := $(PROJDIR)/Scripts/cppcheck

# Default config shared by all
CONFIG = -D${DEVICE}\
         -DHAL_USE_ASSERT=0\
         -DCONFIG_SLEEP=0
         -DCONFIG_BC1P2_CDP=0\
         -DCONFIG_BC1P2_CSM=0\
         -DCONFIG_BC1P2_DCP=0\
         -DCONFIG_BC1P2_DCP_ADDIV=0\
         -DCONFIG_LEGACY_CHARGING=0\

ifeq ($(LOG),yes)
CONFIG += -DCONFIG_LOG=1
else
CONFIG += -DCONFIG_LOG=0
endif

CSTD = c11
# Compiler and linker flags
CFLAGS += -mcpu=$(MCU) \
      -mthumb \
      -std=$(CSTD) \
      $(OPTIMIZE) \
      -g3 \
      -Wall \
      -ffreestanding \
      -ffunction-sections\
      -fdata-sections \
      -ffreestanding \
      -fmessage-length=0 \
      -fsigned-char \
      $(CONFIG)

LDFLAGS += $(CFLAGS) \
       -T$(LDSCRIPT) \
       -nostartfiles \
       -Wl,-Map=$(BUILDDIR)/$(PROJECT).map \
       --specs=nano.specs \
       -Xlinker \
       --gc-sections

LDLIBS += -lgcc -lc


WORKDIR := ${PWD}
#$(PROJDIR)/Examples/$(PROJECT)/usbpd
INCDIR += -I$(WORKDIR)
CSRC += $(wildcard $(WORKDIR)/*.c)
VIF_FILE := $(WORKDIR)/vif_info.h

include $(PROJDIR)/proj.mk
