CFG_PATH := $(PROJDIR)/Scripts/make
VIF_PATH := $(PROJDIR)/Scripts/vif
RELEASEDIR := $(PROJDIR)/Releases

VIF_GEN   := python3 $(PROJDIR)/Utilities/python/xml2header.py
VIF_PATCH := python3 $(PROJDIR)/Utilities/python/vif_patch.py
BUILD_ALL := python3 $(PROJDIR)/Utilities/python/build_all.py
COV_TOOL  := python3 $(PROJDIR)/Utilities/python/test_cov.py
PACKCHK   := $(PROJDIR)/Utilities/pack/PackChk
SVDCONV   := $(PROJDIR)/Utilities/pack/SVDConv

# Define if Verbose build
ifdef V
    VERBOSE = $(V)
else
    VERBOSE = 0
endif

# Select the build thad variant that is passed or default
ifeq ($(BUILD),)
BUILD := default
endif

ifeq ("$(wildcard $(CFG_PATH)/$(BUILD).mk)","")
$(error Not a valid defined build)
endif
include $(CFG_PATH)/$(BUILD).mk

ifeq ($(VARIANT),)
VARIANT := default
endif

ifeq ($(LOG),yes)
include $(PROJDIR)/Drivers/RTT/rtt.mk
endif
include $(PROJDIR)/CMSIS/cmsis.mk
include $(PROJDIR)/External/external.mk
include $(PROJDIR)/Drivers/$(DEVICE)/hal.mk
include $(PROJDIR)/Device/$(DEVICE)/device.mk

# Target files
OUTFILES = $(BUILDDIR)/$(PROJECT).elf \
	   $(BUILDDIR)/$(PROJECT).bin \
	   $(BUILDDIR)/$(PROJECT).hex \
	   $(BUILDDIR)/$(PROJECT).list \
	   $(BUILDDIR)/$(PROJECT).dmp \

# Create Object files
SRCS = $(CSRC) $(CPPSRC) $(ASSRC)
OBJS =  $(subst $(PROJDIR), $(OBJDIR), $(CSRC:.c=.o)) \
    $(subst $(PROJDIR), $(OBJDIR), $(CPPSRC:.cpp=.o))\
	$(subst $(PROJDIR), $(OBJDIR), $(ASSRC:.s=.o))\

.PHONY: all
.PHONY: dmp
.PHONY: clean
.PHONY: distclean
.PHONY: help
.PHONY: doc
.PHONY: test
.PHONY: copy
.PHONY: cppcheck
.PHONY: $(VIF_FILE)

all: $(OBJS) $(OUTFILES)

$(VIF_FILE): $(VIF_PATH)/$(VARIANT).xml
	@echo "Generating VIF file"
ifeq ($(VERBOSE),yes)
	$(VIF_GEN) -v -i $< -o $@
	$(VIF_PATCH) --verbose -v=$(VIF_FILE) -s=$(VIF_STRUCT)
else
	$(VIF_GEN) -i $< -o $@
	$(VIF_PATCH) -v=$(VIF_FILE) -s=$(VIF_STRUCT)
endif

$(OBJS): $(VIF_FILE) $(BUILDDIR) $(OBJDIR)

$(BUILDDIR):
	@mkdir -p $(BUILDDIR)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(BUILDDIR)/$(PROJECT).elf : $(OBJS)
	@$(LD) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@

$(OBJDIR)/%.o : $(PROJDIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCDIR) -c $< -o $@

$(OBJDIR)/%.o : $(PROJDIR)/%.cpp
	@echo $@
	@mkdir -p $(@D)
	$(CPP) $(CFLAGS) $(INCDIR) -c $< -o $@

$(OBJDIR)/%.o : $(PROJDIR)/%.s
	@mkdir -p $(@D)
	$(CPP) $(ASFLAGS) $(INCDIR) -c $< -o $@

%.hex: %.elf
	$(OBJCOPY) -O ihex $< $@

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

%.list: %.elf
	$(OBJDUMP) -S $< > $@

%.dmp: %.elf
	$(OBJSIZE) $<
	@$(OBJSIZE) $< > $@

doc:
	$(MAKE) -C $(DOCDIR)

# Run cppcheck in verbose mode
database:
	@bear make -j4

cppcheck: database $(BUILDDIR)
	@cppcheck --enable=all \
		--template=gcc \
		--inline-suppr \
		--project=compile_commands.json \
		--suppressions-list=$(CPPCHECKDIR)/suppress.txt \
		-D__GNUC__ $(INCDIR) $(SRCS)


# test target is to build a variants and produce binary
# and copy the file to output directory
test: all copy

copy: $(OUTFILES)
	$(eval OUTPATH=$(TESTDIR)/$(BUILD)_$(VARIANT))
	@test -d $(OUTPATH) || mkdir -p $(OUTPATH)
	@for d in $(OUTFILES); do\
		cp $$d $(OUTPATH);\
	done
	@cp $(VIF_PATH)/$(VARIANT).txt $(OUTPATH)
	@cp $(VIF_PATH)/$(VARIANT).xml $(OUTPATH)

# runs external tools that builds all BUILDs and variants
build_all:
	$(BUILD_ALL) -j$(shell nproc) $(BUILD_ALL_ARGS)

clean:
	@echo Cleaning
ifeq ($(VERBOSE),yes)
	@echo $(OUTFILES) $(OBJS)
endif
	@rm -rf $(BUILDDIR)

distclean: clean
	@$(MAKE) -C $(DOCDIR) clean
	@$(MAKE) -C $(PROJDIR)/Packs/$(PROJECT) clean
	@rm -rf $(CPPCHECKDIR)/compile_commands.json

mrproper: distclean
	@rm -rf $(VIF_FILE)
	@rm -rf $(TESTDIR)
	@git checkout $(PROJDIR)

coverage: $(BUILDDIR)
	$(COV_TOOL) -d $(VIF_PATH) -o $(BUILDDIR)/coverage.csv

pack: $(BUILDDIR)
	$(MAKE) -C $(PROJDIR)/Packs/$(PROJECT)
	find $(PROJDIR)/Packs/$(PROJECT) -name "*.pack" -exec cp {} $(BUILDDIR) \;

REPORT_FILE = $(RELEASEDIR)/report-v$(VERSION).xlsx
PACK_FILE = $(BUILDDIR)/ONSemiconductor.$(PROJECT).$(VERSION).pack
release: mrproper pack coverage
	@cp $(BUILDDIR)/coverage.csv $(RELEASEDIR)/coverage-v$(VERSION).csv
	@cp $(PACK_FILE) $(RELEASEDIR)
	@test -e $(REPORT_FILE) || echo "WARNING: $(REPORT_FILE) not found"

help:
	@echo "========================================"
	@echo "make"
	@echo "========================================"
	@echo "make V={yes,no} BUILD={BUILD} VARIANT={VARIANT}"
	@echo "========================================"
	@echo "make clean"
	@echo "========================================"
	@echo "make distclean"
	@echo "========================================"
	@echo "make mrproper"
	@echo "========================================"
	@echo "make build_all"
	@echo "========================================"
	@echo "make test"
	@echo "========================================"
	@echo "make coverage"
	@echo "========================================"
	@echo "make cppcheck"
	@echo "========================================"
	@echo "make pack"
	@echo "========================================"
	@echo "make release"
	@echo "========================================"
