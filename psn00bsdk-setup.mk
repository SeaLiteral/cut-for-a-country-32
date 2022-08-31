# PSn00bSDK project setup file
# Part of the PSn00bSDK Project
# 2019 - 2021 Lameguy64 / Meido-Tek Productions
#
# This file does not depend on any other files (besides paths specified via
# environment variables) and may be copied for use with your projects. See the
# template directory for a makefile template.
#
# Modified by Lars Rune Præstmark to work with multi-target makefiles

# NOTE: This file is meant to be one directory higher-up than the game itself.

#PREFIX	?= mipsel-none-elf
PSNOOB_PREFIX	?= mipsel-none-elf

## Path setup

# PSn00bSDK library/include path setup
ifndef PSN00BSDK_LIBS
	# Default assumes PSn00bSDK is in the same parent dir as this project
	PSNOOB_LIBDIRS	= -L../libpsn00b
	PSNOOB_INCLUDE	= -I../libpsn00b/include -I../libpsn00b/lzp
	PSNOOB_LDBASE	= ../libpsn00b/ldscripts
else
	PSNOOB_LIBDIRS	= -L$(PSN00BSDK_LIBS)
	PSNOOB_INCLUDE	= -I$(PSN00BSDK_LIBS)/include -I$(PSN00BSDK_LIBS)/lzp
	PSNOOB_LDBASE	= ${PSN00BSDK_LIBS}/ldscripts
endif

# PSn00bSDK toolchain path setup
ifndef PSN00BSDK_TC
	# Default assumes GCC toolchain is in root of C drive or /usr/local
	ifeq "$(OS)" "Windows_NT"
		PSNOOB_GCC_BASE	?= /c/$(PSNOOB_PREFIX)
		PSNOOB_GCC_BIN		?=
	else
		PSNOOB_GCC_BASE	?= /usr/local/$(PSNOOB_PREFIX)
		PSNOOB_GCC_BIN		?=
	endif
else
	PSNOOB_GCC_BASE	?= $(PSN00BSDK_TC)
	PSNOOB_GCC_BIN		?= 
	#$(PSN00BSDK_TC)/bin/
endif

# Autodetect GCC version by folder name (ugly but it works, lol)
#GCC_VERSION	?= 7.4.0
PSNOOB_GCC_VERSION	?= $(word 1, $(notdir $(wildcard $(PSNOOB_GCC_BASE)/lib/gcc/$(PSNOOB_PREFIX)/*)))

# PSn00bSDK tools path setup (TODO)
PSN00BSDK_BIN	?=

## Commands

# GCC toolchain
PSNOOB_CC	= $(PSNOOB_GCC_BIN)$(PSNOOB_PREFIX)-gcc
PSNOOB_CXX	= $(PSNOOB_GCC_BIN)$(PSNOOB_PREFIX)-g++
PSNOOB_AS	= $(PSNOOB_GCC_BIN)$(PSNOOB_PREFIX)-as
PSNOOB_AR	= $(PSNOOB_GCC_BIN)$(PSNOOB_PREFIX)-ar
PSNOOB_LD	= $(PSNOOB_GCC_BIN)$(PSNOOB_PREFIX)-ld
PSNOOB_RANLIB	= $(PSNOOB_GCC_BIN)$(PSNOOB_PREFIX)-ranlib
PSNOOB_OBJCOPY	= $(PSNOOB_GCC_BIN)$(PSNOOB_PREFIX)-objcopy
PSNOOB_NM	= $(PSNOOB_GCC_BIN)$(PSNOOB_PREFIX)-nm

# PSn00bSDK tools + mkpsxiso
ELF2X		= $(PSN00BSDK_BIN)elf2x
LZPACK		= $(PSN00BSDK_BIN)lzpack
SMXLINK		= $(PSN00BSDK_BIN)smxlink
MKPSXISO	= $(PSN00BSDK_BIN)mkpsxiso

## Flags

# SDK libraries (IMPORTANT: don't change the order)
PSNOOB_LIBS	= -lpsxgpu -lpsxgte -lpsxspu -lpsxcd -lpsxsio -lpsxetc -lpsxapi -lc

# Common options:
# - Debugging symbols enabled
# - Wrap each symbol in a separate section
# - Optimize for R3000, no FPU, 32-bit ABI
# - Division by zero causes break opcodes to be executed
# - C standard library (including libgcc) disabled
# - C++ features that rely on runtime support disabled
PSNOOB_AFLAGS	= -g -msoft-float -march=r3000 -mtune=r3000 -mabi=32
PSNOOB_CFLAGS	= $(PSNOOB_AFLAGS) -mdivide-breaks -O2 -ffreestanding -fno-builtin -nostdlib \
		-fdata-sections -ffunction-sections -fsigned-char -fno-strict-overflow
PSNOOB_CPPFLAGS= $(PSNOOB_CFLAGS) -fno-exceptions -fno-rtti -fno-unwind-tables \
		-fno-threadsafe-statics -fno-use-cxa-atexit
PSNOOB_LDFLAGS	= -nostdlib

# Options for static libraries (and SDK libraries):
# - GP-relative addressing disabled
# - ABI-compatible calls disabled
# - Local stripping enabled
PSNOOB_AFLAGS_LIB	= $(PSNOOB_AFLAGS) -G0 -Wa,--strip-local-absolute
PSNOOB_CFLAGS_LIB	= $(PSNOOB_CFLAGS) -G0 -mno-abicalls -mno-gpopt
PSNOOB_CPPFLAGS_LIB	= $(PSNOOB_CPPFLAGS) -G0 -mno-abicalls -mno-gpopt

# Options for executables without support for dynamic linking:
# - Position-independent code disabled
# - GP-relative addressing enabled only for local symbols
# - ABI-compatible calls disabled (incompatible with GP-relative addressing)
# - Unused section stripping enabled
PSNOOB_AFLAGS_EXE	= $(PSNOOB_AFLAGS) -G8
PSNOOB_CFLAGS_EXE	= $(PSNOOB_CFLAGS) -G8 -mno-abicalls -mgpopt -mno-extern-sdata
PSNOOB_CPPFLAGS_EXE	= $(PSNOOB_CPPFLAGS) -G8 -mno-abicalls -mgpopt -mno-extern-sdata
PSNOOB_LDFLAGS_EXE	= $(PSNOOB_LDFLAGS) -G8 -static -T$(PSNOOB_LDBASE)/exe.ld -gc-sections

# Options for executables with support for dynamic linking:
# - Position-independent code disabled
# - GP-relative addressing disabled
# - ABI-compatible calls disabled (must be done manually)
# - Unused section stripping enabled
PSNOOB_AFLAGS_EXEDYN	= $(PSNOOB_AFLAGS) -G0
PSNOOB_CFLAGS_EXEDYN	= $(PSNOOB_CFLAGS) -G0 -mno-abicalls -mno-gpopt
PSNOOB_CPPFLAGS_EXEDYN	= $(PSNOOB_CPPFLAGS) -G0 -mno-abicalls -mno-gpopt
PSNOOB_LDFLAGS_EXEDYN	= $(PSNOOB_LDFLAGS) -G0 -static -T$(PSNOOB_LDBASE)/exe.ld -gc-sections

# Options for dynamically-loaded libraries:
# - Position-independent code enabled
# - GP-relative addressing disabled (incompatible with ABI calls)
# - ABI-compatible calls enabled
# - Unused section stripping not available
PSNOOB_AFLAGS_DLL	= $(PSNOOB_AFLAGS) -G0
PSNOOB_CFLAGS_DLL	= $(PSNOOB_CFLAGS) -G0 -mabicalls -mshared -mno-gpopt -fPIC
PSNOOB_CPPFLAGS_DLL	= $(PSNOOB_CPPFLAGS) -G0 -mabicalls -mshared -mno-gpopt -fPIC
PSNOOB_LDFLAGS_DLL	= $(PSNOOB_LDFLAGS) -G0 -shared -T$(PSNOOB_LDBASE)/dll.ld
