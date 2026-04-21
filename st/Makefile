#MATIEC = $(HOME)/matiec
MATIEC = ../matiec/matiec
# mentioned in matiec_examples but not really used by matiec
#export MATIEC_INCLUDE_PATH = $(MATIEC)/lib
#export MATIEC_C_INCLUDE_PATH = $(MATIEC)/lib/C
export PATH := $(PATH):$(MATIEC)/../build

MATIEC_OPTIONS += -I $(MATIEC)/lib
# -c : create conversion functions for enumerated data types
MATIEC_OPTIONS += -c
#      l : insert '#line' directives in generated C code.
#      p : place each POU in a separate pair of files (<pou_name>.c, <pou_name>.h).
#      b : generate functions to backup and restore internal PLC state.
#MATIEC_OPTIONS += -O l,p
MATIEC_OPTIONS += -O l

# LB241 by default; for LB340, overridden with make ARCH=xtensa-esp32-elf
ARCH=xtensa-esp32s3-elf
CC=$(ARCH)-gcc
SIZE=$(ARCH)-size
STRIP=$(ARCH)-strip
OBJCOPY=$(ARCH)-objcopy
OBJDUMP=$(ARCH)-objdump

CFILES=build/Res[0-9].c build/Config[0-9].c

#CPPFLAGS=-I$(MATIEC)/lib/C -Wall -fpic -g
CPPFLAGS+=-I. # for <accessor.h>
CPPFLAGS+=-I$(MATIEC)/lib/C

# https://esp32.com/viewtopic.php?t=4293
CFLAGS+=-g
CFLAGS+=-Wall
CFLAGS+=-fno-function-sections
CFLAGS+=-fno-data-sections
#CFLAGS+=-fsingle-precision-constant
CFLAGS+=-mtext-section-literals
CFLAGS+=-mlongcalls  # adds R_XTENSA_ASM_EXPAND... no problem for GNU ld
CFLAGS+=-Os # size 20x less than -O0
## https://gcc.gnu.org/onlinedocs/gcc/Xtensa-Options.html
CFLAGS+=-mauto-litpools # gets rid of .literal section
#CFLAGS+=-rdynamic
CFLAGS+=-Wno-unused-function # lots of these in matiec/lib/C/iec_std_FB.h

REMOVE_UNNEEDED_SECTIONS+=--remove-section=.xt.lit
REMOVE_UNNEEDED_SECTIONS+=--remove-section=.xt.prop
REMOVE_UNNEEDED_SECTIONS+=--remove-section=.comment
REMOVE_UNNEEDED_SECTIONS+=--remove-section=.xtensa.info

LDFLAGS+=-T build/linker-script.ld
LDFLAGS+=-nostartfiles
LDFLAGS+=-nodefaultlibs
LDFLAGS+=-u headers
LDFLAGS+=-Xlinker -Map=build/map.txt

LIBS+=-lm -lc

SECTIONS_HEADER_TEXT+=--only-section=.theader
SECTIONS_HEADER_TEXT+=--only-section=.text

SECTION_DATA+=--only-section=.data
SECTION_DATA+=--only-section='.rodata*'

compile: build/merged.c

binary: build/text.bin build/data.bin

.PHONY: build/merged.c
build/merged.c: $(SRC)
	test -f $(SRC)
	rm -f build/*
	mkdir -p build
	iec2c -T build/ $(MATIEC_OPTIONS) $(SRC)
	cat build/POUS.h build/POUS.c > build/merged.c

build/merged.o: build/merged.c force
	$(CC) $(CPPFLAGS) $(CFLAGS) -c build/merged.c -o build/merged.o
	$(OBJCOPY) $(REMOVE_UNNEEDED_SECTIONS) build/merged.o

# build tentatively with fictituous addresses,
# just to determine the segment sizes
.PHONY: size
size:
	$(MAKE) TEXT_START=0x42880000 \
	        DATA_START=0x3c7e0000 \
	         BSS_START=0x3c7f0000 binary
	./size-from-map.pl ./build/map.txt

.PHONY: build/linker-script.ld
build/linker-script.ld:
	./mk-linker-script-$(ARCH).pl\
	  $(TEXT_START) $(DATA_START) $(BSS_START) > $@.new
	mv $@.new $@

build/headers.c: mk-headers.pl build/POUS.c
	./mk-headers.pl build/POUS.c > $@.new
	mv $@.new $@

build/text.bin build/data.bin: build/merged.o build/headers.c build/linker-script.ld
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -c build/headers.c -o build/headers.o
	$(OBJCOPY) $(REMOVE_UNNEEDED_SECTIONS) build/headers.o
	$(CC) $(LDFLAGS) build/headers.o build/merged.o $(LIBS) -o build/merged.elf
	$(OBJCOPY) -O binary $(SECTIONS_HEADER_TEXT) build/merged.elf build/text.bin
	$(OBJCOPY) -O binary $(SECTION_DATA) build/merged.elf build/data.bin

clean:
	rm -rf build

force:
