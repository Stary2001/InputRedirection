#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITARM)/3ds_rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# DATA is a list of directories containing data files
# INCLUDES is a list of directories containing header files
#
# NO_SMDH: if set to anything, no SMDH file is generated.
# APP_TITLE is the name of the app stored in the SMDH file (Optional)
# APP_DESCRIPTION is the description of the app stored in the SMDH file (Optional)
# APP_AUTHOR is the author of the app stored in the SMDH file (Optional)
# ICON is the filename of the icon (.png), relative to the project folder.
#   If not set, it attempts to use one of the following (in this order):
#     - <Project name>.png
#     - icon.png
#     - <libctru folder>/default_icon.png
#---------------------------------------------------------------------------------
TARGET		:=	$(notdir $(CURDIR))
BUILD		:=	build
SOURCES		:=	source
DATA		:=	data
INCLUDES	:=	include
APP_TITLE	:=	InputProc
APP_TITLE_MODE3	:= InputProc-Mode3
APP_DESCRIPTION := "Patches HID/IR for use with InputRedirection."
APP_AUTHOR	:=	Stary
ICON		:=	meta/icon.png
DO_3DSX		:=	no

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-march=armv6k -mtune=mpcore -mfloat-abi=hard

CFLAGS	:=	-g -Wall -Os -mword-relocations \
			-fomit-frame-pointer -ffast-math -ffunction-sections -flto \
			$(ARCH)

LIBS	:= -lctru -lm -lscenic

CFLAGS	+=	$(INCLUDE) -DARM11 -D_3DS -DARM_ARCH -w

CXXFLAGS	:= $(CFLAGS) -fno-rtti -std=gnu++11 -w

ASFLAGS	:=	-g $(ARCH) $(INCLUDE)
LDFLAGS	=	-specs=3dsx.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map) -Wl,--gc-sections -flto $(LIBS)

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= $(CTRULIB)


#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export TOPDIR	:=	$(CURDIR)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
			$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

ifeq ($(strip $(ICON)),)
	icons := $(wildcard *.png)
	ifneq (,$(findstring $(TARGET).png,$(icons)))
		export APP_ICON := $(TOPDIR)/$(TARGET).png
	else
		ifneq (,$(findstring icon.png,$(icons)))
			export APP_ICON := $(TOPDIR)/icon.png
		endif
	endif
else
	export APP_ICON := $(TOPDIR)/$(ICON)
endif

.PHONY: $(BUILD) clean all

#---------------------------------------------------------------------------------
all: $(BUILD)

$(BUILD):
	@echo $(SFILES)
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	rm -fr $(BUILD) $(OUTPUT).3dsx $(OUTPUT).smdh $(OUTPUT).elf $(OUTPUT)_stripped.elf $(OUTPUT).cia $(OUTPUT)_mode3.cia

#---------------------------------------------------------------------------------
send: $(BUILD)
	3dslink -a 192.168.0.4 $(TARGET).3dsx

#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
ifeq ($(strip $(DO_3DSX)), yes)
.PHONY: all
all	:	$(OUTPUT).3dsx $(OUTPUT).smdh $(OUTPUT).cia
else
.PHONY: all
all    :       $(OUTPUT).cia $(OUTPUT)_mode3.cia
endif

$(OUTPUT).3dsx	:	$(OUTPUT).elf
$(OUTPUT).elf	:	$(OFILES)

icon.icn: $(TOPDIR)/meta/icon.png
	@bannertool makesmdh -i $(TOPDIR)/meta/icon.png -s $(APP_TITLE) -l $(APP_DESCRIPTION) -p $(APP_AUTHOR) -o icon.icn

banner.bnr: $(TOPDIR)/meta/banner.png $(TOPDIR)/meta/audio.wav
	@bannertool makebanner -i $(TOPDIR)/meta/banner.png -a $(TOPDIR)/meta/audio.wav -o banner.bnr

icon_mode3.icn: $(TOPDIR)/meta/icon_mode3.png
	@bannertool makesmdh -i $(TOPDIR)/meta/icon_mode3.png -s $(APP_TITLE_MODE3) -l $(APP_DESCRIPTION) -p $(APP_AUTHOR) -o icon_mode3.icn

banner_mode3.bnr: $(TOPDIR)/meta/banner_mode3.png $(TOPDIR)/meta/audio.wav
	@bannertool makebanner -i $(TOPDIR)/meta/banner_mode3.png -a $(TOPDIR)/meta/audio.wav -o banner_mode3.bnr

$(OUTPUT)_stripped.elf: $(OUTPUT).elf
	@cp $(OUTPUT).elf $(OUTPUT)_stripped.elf
	@$(PREFIX)strip $(OUTPUT)_stripped.elf

$(OUTPUT).cia: $(OUTPUT)_stripped.elf banner.bnr icon.icn
	@makerom -f cia -o $(OUTPUT).cia -rsf $(TOPDIR)/meta/cia.rsf -target t -exefslogo -elf $(OUTPUT)_stripped.elf -icon icon.icn -banner banner.bnr -ver 1040
	@echo "built ... $(notdir $@)"

$(OUTPUT)_mode3.cia: $(OUTPUT)_stripped.elf banner_mode3.bnr icon_mode3.icn
	@makerom -f cia -o $(OUTPUT)_mode3.cia -rsf $(TOPDIR)/meta/cia_mode3.rsf -target t -exefslogo -elf $(OUTPUT)_stripped.elf -icon icon_mode3.icn -banner banner_mode3.bnr -ver 1040
	@echo "built ... $(notdir $@)"

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------