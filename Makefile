# Aurora Ticket (JPAJ) distribution cartridge.
#
#   make           build aurora_ticket.gba
#   make compare   build it and check it matches the original
#
# Needs arm-none-eabi binutils and agbcc, installed in tools/agbcc (see
# README.md).

PREFIX   ?= arm-none-eabi-
AS       := $(PREFIX)as
LD       := $(PREFIX)ld
AR       := $(PREFIX)ar
OBJCOPY  := $(PREFIX)objcopy
CPP      := cpp
PYTHON   ?= python3
CC1      := tools/agbcc/bin/agbcc

ASFLAGS  := -mcpu=arm7tdmi -I .
CPPFLAGS := -nostdinc -undef -I include -I tools/agbcc/include -iquote include
CFLAGS   := -mthumb-interwork -Wimplicit -Wparentheses -O2 -fhex-asm

BUILD    := build
ROM      := aurora_ticket.gba
ELF      := $(ROM:.gba=.elf)

include objs.mk

.PHONY: all compare clean
.DELETE_ON_ERROR:

all: $(ROM)

compare: $(ROM)
	@sha1sum -c aurora_ticket.sha1

clean:
	rm -rf $(BUILD) $(ROM) $(ELF) $(ROM:.gba=.map)
	rm -f $(GFX_1BPP) $(GFX_4BPP) $(GFX_8BPP) $(GFX_PAL)

HEADERS := $(wildcard include/*.h include/*/*.h)

$(BUILD)/%.o: %.c $(HEADERS)
	@mkdir -p $(@D)
	$(CPP) $(CPPFLAGS) $< -o $(@:.o=.i)
	$(CC1) $(CFLAGS) $(@:.o=.i) -o $(@:.o=.s)
	printf '\t.text\n\t.align 2, 0 @ pad with 0, not nop\n' >> $(@:.o=.s)
	$(AS) $(ASFLAGS) $(@:.o=.s) -o $@

$(BUILD)/%.o: %.s asm/macros/function.inc
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) $< -o $@

# The data's text is encoded with charmap.txt, and its graphics are built from
# graphics/.
incbins = $(shell sed -n 's/^[[:space:]]*\.incbin[[:space:]]*"\([^"]*\)".*/\1/p' $(1))
INCBINS := $(call incbins,$(wildcard data/*.s))

.SECONDEXPANSION:
$(BUILD)/data/%.o: data/%.s charmap.txt tools/preproc.py $(wildcard asm/macros/*.inc) $$(call incbins,data/$$*.s)
	@mkdir -p $(@D)
	$(PYTHON) tools/preproc.py $< charmap.txt > $(@:.o=.s)
	$(AS) $(ASFLAGS) $(@:.o=.s) -o $@

GFX_1BPP := $(filter %.1bpp,$(INCBINS))
GFX_4BPP := $(filter %.4bpp,$(INCBINS))
GFX_8BPP := $(filter %.8bpp,$(INCBINS))
GFX_PAL  := $(patsubst %.pal,%.gbapal,$(wildcard graphics/*/*.pal))

$(GFX_1BPP): %.1bpp: %.png tools/gbagfx.py
	$(PYTHON) tools/gbagfx.py $< $@ $(GFXFLAGS)
$(GFX_4BPP): %.4bpp: %.png tools/gbagfx.py
	$(PYTHON) tools/gbagfx.py $< $@ $(GFXFLAGS)
$(GFX_8BPP): %.8bpp: %.png tools/gbagfx.py
	$(PYTHON) tools/gbagfx.py $< $@ $(GFXFLAGS)
$(GFX_PAL): %.gbapal: %.pal tools/gbagfx.py
	$(PYTHON) tools/gbagfx.py $< $@

# The font's 16x16 characters are 2x2 tiles.
graphics/font/font.1bpp: GFXFLAGS := -mwidth 2 -mheight 2

# librfu's interrupt handlers are ARM code.
$(BUILD)/src/librfu_intr.o: CC1 := tools/agbcc/bin/agbcc_arm
$(BUILD)/src/librfu_intr.o: CFLAGS := -O2 -mthumb-interwork -quiet
# The flash library is built with -O, the RTC library without optimization.
$(BUILD)/src/agb_flash.o: CFLAGS := -O -mthumb-interwork -fhex-asm
$(BUILD)/src/siirtc.o: CFLAGS := -mthumb-interwork -fhex-asm

# The cartridge was linked with agbcc's libc, its objects sorted by name (which
# decides the order the linker pulls them in).
$(BUILD)/libc.a: tools/agbcc/lib/libc.a
	rm -rf $(@D)/libc $@
	mkdir -p $(@D)/libc
	cd $(@D)/libc && $(AR) x $(CURDIR)/$<
	cd $(@D)/libc && $(AR) rcs $(CURDIR)/$@ $$(ls *.o | LC_ALL=C sort)
	rm -rf $(@D)/libc

$(ELF): $(addprefix $(BUILD)/,$(OBJS)) $(BUILD)/libc.a ld/aurora.ld ld/syms.ld
	$(LD) -L ld -T ld/aurora.ld -Map $(ROM:.gba=.map) -o $@ $(filter %.o,$^) tools/agbcc/lib/libgcc.a $(BUILD)/libc.a tools/agbcc/lib/libgcc.a

$(ROM): $(ELF)
	$(OBJCOPY) -O binary --pad-to 0x8400000 --gap-fill 0xFF $< $@
