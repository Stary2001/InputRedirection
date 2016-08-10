# TARGET #

TARGET := 3DS
LIBRARY := 0

ifeq ($(TARGET),3DS)
    ifeq ($(strip $(DEVKITPRO)),)
        $(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>devkitPro")
    endif

    ifeq ($(strip $(DEVKITARM)),)
        $(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
    endif
endif

# COMMON CONFIGURATION #

NAME := InputProc

BUILD_DIR := build
OUTPUT_DIR := output
INCLUDE_DIRS := include
SOURCE_DIRS := source

EXTRA_OUTPUT_FILES :=

LIBRARY_DIRS := $(DEVKITPRO)/libctru
LIBRARIES := ctru m

BUILD_FLAGS :=
RUN_FLAGS :=

# 3DS CONFIGURATION #

TITLE := $(NAME) $(TYPE)
DESCRIPTION := Relays input for the wifi thing. Go away.
AUTHOR := Stary
PRODUCT_CODE := CTR-P-CINP
UNIQUE_ID := 0x96e7f

SYSTEM_MODE := 64MB
SYSTEM_MODE_EXT := Legacy

ICON_FLAGS := --flags visible,ratingrequired,recordusage --cero 153 --esrb 153 --usk 153 --pegigen 153 --pegiptr 153 --pegibbfc 153 --cob 153 --grb 153 --cgsrr 153

ROMFS_DIR := romfs
BANNER_AUDIO := meta/audio.wav
BANNER_IMAGE := meta/banner.png
ICON := meta/icon_hbl.png

#all:

# INTERNAL #

include buildtools/make_base

clean_ntr:
	make clean BUILD_DIR=build_ntr OUTPUT_DIR=output_ntr

ntr:
	sed -i 's/MemoryType                    : Base/MemoryType                    : Application/' buildtools/3ds/template.rsf
	make BUILD_DIR=build_ntr OUTPUT_DIR=output_ntr BUILD_FLAGS=-DNTR UNIQUE_ID=0x96e7e ICON=meta/icon_ntr.png TYPE=NTR

hbl:
	sed -i 's/MemoryType                    : Application/MemoryType                    : Base/' buildtools/3ds/template.rsf
	make TYPE=HBL