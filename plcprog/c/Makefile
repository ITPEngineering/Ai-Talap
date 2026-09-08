SRCS := $(wildcard src/*.c)
OBJS := $(patsubst %.c,$(BUILD)/%.o,$(notdir $(SRCS)))
SCRIPTS=./scripts

# LB241 by default; for LB340, overridden with make ARCH=xtensa-esp32-elf
ARCH=xtensa-esp32s3-elf
CC=$(ARCH)-gcc
SIZE=$(ARCH)-size
STRIP=$(ARCH)-strip
OBJCOPY=$(ARCH)-objcopy
OBJDUMP=$(ARCH)-objdump

CPPFLAGS+=-I./include # for logicbox.h etc.

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

LDFLAGS+=-T $(BUILD)/linker-script.ld
LDFLAGS+=-nostartfiles
#LDFLAGS+=-nodefaultlibs
LDFLAGS+=-u headers
LDFLAGS+=-Xlinker -Map=$(BUILD)/map.txt

LIBS+=-lm -lc

SECTIONS_HEADER_TEXT+=--only-section=.theader
SECTIONS_HEADER_TEXT+=--only-section=.text

SECTION_DATA+=--only-section=.data
SECTION_DATA+=--only-section='.rodata*'

binary: $(BUILD)/text.bin $(BUILD)/data.bin

$(BUILD)/compiled.o: $(SRCFILE) force
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
	$(OBJCOPY) $(REMOVE_UNNEEDED_SECTIONS) $@

$(BUILD)/%.o: src/%.c force
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
	$(OBJCOPY) $(REMOVE_UNNEEDED_SECTIONS) $@

$(BUILD)/linker-script.ld: force
	$(SCRIPTS)/mk-linker-script-$(ARCH).pl\
	  $(TEXT_START) $(DATA_START) $(BSS_START) > $@.new
	mv $@.new $@

# lbupload needs text and data images separately
$(BUILD)/text.bin $(BUILD)/data.bin: $(OBJS) $(BUILD)/compiled.o $(BUILD)/linker-script.ld
	$(AR) r $(BUILD)/objs.a $(OBJS) $(BUILD)/compiled.o
	$(CC) $(LDFLAGS) $(BUILD)/headers.o $(BUILD)/objs.a $(LIBS) -o $(BUILD)/compiled.elf
	$(OBJCOPY) -O binary $(SECTIONS_HEADER_TEXT) $(BUILD)/compiled.elf $(BUILD)/text.bin
	$(OBJCOPY) -O binary $(SECTION_DATA) $(BUILD)/compiled.elf $(BUILD)/data.bin

# build tentatively with fictituous addresses,
# just to determine the segment sizes
.PHONY: size
size:
	$(MAKE) TEXT_START=0x42880000 \
	        DATA_START=0x3c7e0000 \
	         BSS_START=0x3c7f0000 binary
	$(SCRIPTS)/size-from-map.pl $(BUILD)/map.txt

clean:
	rm -rf $(BUILD)

force:
