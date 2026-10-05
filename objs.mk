# The cartridge's objects, in link order.
OBJS := \
	asm/crt0.o \
	src/save.o \
	src/content.o \
	src/bg.o \
	src/main.o \
	src/rfu.o \
	src/link.o \
	src/setup.o \
	src/misc.o \
	src/AgbRfu_LinkManager.o \
	src/ui.o \
	src/mg.o \
	src/ctrl.o \
	src/librfu_stwi.o \
	src/librfu_intr.o \
	data/event_script.o \
	data/rodata.o \
	data/graphics.o \
	src/librfu_rfu.o \
	src/librfu_sio32id.o \
	src/siirtc.o \
	src/agb_flash.o \
	asm/libagbsyscall.o
