# Based on the PSn00bSDK makefile template
# Part of the PSn00bSDK Project
# 2019 - 2021 Lameguy64 / Meido-Tek Productions

## Settings

# You can edit these here or pass them as environment variables.
#PREFIX			=
#GCC_VERSION	=
#PSN00BSDK_TC	=
#PSN00BSDK_LIBS	=

# Edit this to point to psn00bsdk-setup.mk, or copy that over to your project's
# root folder (it only depends on environment variables).
include ../psn00bsdk-setup.mk

# Project target name
TARGET	= cfac
TARGETN = $(TARGET)n
TARGETP = $(TARGET)p

## Files

# Searches for C, C++ and S (assembler) files in local directory
CFILES	= $(notdir $(wildcard *.c))
CPPFILES= $(notdir $(wildcard *.cpp))
AFILES	= $(notdir $(wildcard *.s))

# Create names for object files

# for NTSC
OFILESN	= $(addprefix buildn/,$(CFILES:.c=.o)) \
	$(addprefix buildn/,$(CPPFILES:.cpp=.o)) \
	$(addprefix buildn/,$(AFILES:.s=.o)) \
	buildn/ui_ps1.o
# and PAL
OFILESP	= $(addprefix buildp/,$(CFILES:.c=.o)) \
	$(addprefix buildp/,$(CPPFILES:.cpp=.o)) \
	$(addprefix buildp/,$(AFILES:.s=.o)) \
	buildp/ui_ps1.o
# Project specific includes and libraries
# (use -I for include dirs, -L for library dirs, -l for static libraries)
INCLUDE	+=
LIBDIRS	+=
LIBS	+=

CFLAGS_NTSC = $(CFLAGS_EXE)
CFLAGS_PAL = $(CFLAGS_EXE) -D MODE_IS_PAL
CPPFLAGS_NTSC = $(CPPFLAGS_EXE)
CPPFLAGS_PAL = $(CPPFLAGS_EXE) -D MODE_IS_PAL

## build rules

all: buildn/$(TARGETN).iso buildp/$(TARGETP).iso
#all: build/$(TARGET)

buildn/$(TARGETN).iso: buildn/$(TARGETN).exe
	$(MKPSXISO) -y -q ison.xml

buildp/$(TARGETP).iso: buildp/$(TARGETP).exe
	$(MKPSXISO) -y -q isop.xml

#resources: build/$(TARGET).exe
#	# Add commands to buildn/convert your assets here
#	#$(LZPACK) data.xml



buildn/$(TARGETN).elf: $(OFILESN) buildn/ui_ps1.o
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS_EXE) $(LIBDIRS) $^ $(LIBS) -o $@
	$(NM) -f posix -l -n $@ >$@.map

buildp/$(TARGETP).elf: $(OFILESP) buildp/ui_ps1.o
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS_EXE) $(LIBDIRS) $^ $(LIBS) -o $@
	$(NM) -f posix -l -n $@ >$@.map



buildn/$(TARGETN).exe: buildn/$(TARGETN).elf
	$(ELF2X) -q buildn/$(TARGETN).elf $@

buildp/$(TARGETP).exe: buildp/$(TARGETP).elf
	$(ELF2X) -q buildp/$(TARGETP).elf $@



buildn/main.o: main.c build-tr/ui_ps1.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_NTSC) $(INCLUDE) -c main.c -o buildn/main.o

buildp/main.o: main.c build-tr/ui_ps1.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_PAL) $(INCLUDE) -c main.c -o buildp/main.o

build-tr/ui_ps1.bin build-tr/ui_ps1.h build-tr/ui_ps1.s: translations/interface.ini
	@mkdir -p $(dir $@)
	python3 convert_strings.py build-tr/ui_ps1

buildn/ui_ps1.o: build-tr/ui_ps1.s
	@mkdir -p $(dir $@)
	$(CC) $(AFLAGS_EXE) $(INCLUDE) -c $< -o $@

buildp/ui_ps1.o: build-tr/ui_ps1.s
	@mkdir -p $(dir $@)
	$(CC) $(AFLAGS_EXE) $(INCLUDE) -c $< -o $@

buildn/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS_NTSC) $(INCLUDE) -c $< -o $@
# In the PAL version, we might want some differences
buildp/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS_PAL) $(INCLUDE) -c $< -o $@




buildn/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(AFLAGS_EXE) $(INCLUDE) -c $< -o $@

buildp/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(AFLAGS_EXE) $(INCLUDE) -c $< -o $@




clean:
	rm -rf buildn
	rm -rf buildp
	rm -rf build-tr
