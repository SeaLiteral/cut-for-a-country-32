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
	$(addprefix buildn/,$(AFILES:.s=.o))
# and PAL
OFILESP	= $(addprefix buildp/,$(CFILES:.c=.o)) \
	$(addprefix buildp/,$(CPPFILES:.cpp=.o)) \
	$(addprefix buildp/,$(AFILES:.s=.o))
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



buildn/$(TARGETN).elf: $(OFILESN)
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS_EXE) $(LIBDIRS) $^ $(LIBS) -o $@
	$(NM) -f posix -l -n $@ >$@.map

buildp/$(TARGETP).elf: $(OFILESP)
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS_EXE) $(LIBDIRS) $^ $(LIBS) -o $@
	$(NM) -f posix -l -n $@ >$@.map



buildn/$(TARGETN).exe: buildn/$(TARGETN).elf
	$(ELF2X) -q buildn/$(TARGETN).elf $@

buildp/$(TARGETP).exe: buildp/$(TARGETP).elf
	$(ELF2X) -q buildp/$(TARGETP).elf $@



buildn/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_NTSC) $(INCLUDE) -c $< -o $@

buildp/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_PAL) $(INCLUDE) -c $< -o $@



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
