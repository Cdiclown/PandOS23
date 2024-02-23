# Makefile based on the one found at <https://wiki.virtualsquare.org/education/tutorials/umps/examples/hello-world.tar.gz> on the VirtualSquare Wiki

# Cross toolchain variables
# If these are not in your path, you can make them absolute.
XT_PRG_PREFIX = mipsel-linux-gnu-
CC = $(XT_PRG_PREFIX)gcc
LD = $(XT_PRG_PREFIX)ld

# uMPS3-related paths

# Simplistic search for the umps3 installation prefix.
# If you have umps3 installed on some weird location, set UMPS3_DIR_PREFIX by hand.
ifneq ($(wildcard /usr/bin/umps3),)
	UMPS3_DIR_PREFIX = /usr
else
	UMPS3_DIR_PREFIX = /usr/local
endif

UMPS3_DATA_DIR = $(UMPS3_DIR_PREFIX)/share/umps3
UMPS3_INCLUDE_DIR = $(UMPS3_DIR_PREFIX)/include/umps3
UMPS3_INCLUDE_DIR_2 = $(UMPS3_DIR_PREFIX)/include

# Compiler options
CFLAGS_LANG = -ffreestanding -ansi
CFLAGS_MIPS = -mips1 -mabi=32 -mno-gpopt -EL -G 0 -mno-abicalls -fno-pic -mfp32
CFLAGS = $(CFLAGS_LANG) $(CFLAGS_MIPS) -I$(UMPS3_INCLUDE_DIR) -I$(UMPS3_INCLUDE_DIR_2) -std=gnu99 -Wall -O0

# Linker options
LDFLAGS = -G 0 -nostdlib -T $(UMPS3_DATA_DIR)/umpscore.ldscript -m elf32ltsmip

# Add the location of crt*.S to the search path
VPATH = $(UMPS3_DATA_DIR)

# Phase 1 files
SRC_FILES = $(wildcard phase1/*.c) $(wildcard phase2/*.c) phase2/test/p2test.c
OBJ_FILES = $(SRC_FILES:.c=.o)

.PHONY : all clean

all : bin/kernel.core.umps

bin/kernel.core.umps : bin/kernel
	umps3-elf2umps -k $<

bin/kernel : $(OBJ_FILES) bin/crtso.o bin/libumps.o
	$(LD) -o $@ $^ $(LDFLAGS)

clean :
	-rm -rf bin $(OBJ_FILES)

# Pattern rule for assembly modules
bin/%.o : %.S
	mkdir -p bin
	$(CC) $(CFLAGS) -c -o $@ $<
