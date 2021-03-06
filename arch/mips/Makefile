#
# Select the object file format to substitute into the linker script.
#
ifdef CONFIG_CPU_LITTLE_ENDIAN
32bit-emul		= elf32ltsmip
64bit-emul		= elf64ltsmip
else
32bit-emul		= elf32btsmip
64bit-emul		= elf64btsmip
endif

CPPFLAGS += -D__MIPS__ -fno-strict-aliasing -fno-merge-constants

cflags-y += -G 0 -mno-abicalls -fno-pic -pipe
cflags-y += -Wall -Wmissing-prototypes -Wstrict-prototypes \
	-Wno-uninitialized -Wno-format -Wno-main

ifdef CONFIG_32BIT
ld-emul		= $(32bit-emul)
cflags-y	+= -mabi=32
endif

ifdef CONFIG_64BIT
ld-emul		= $(64bit-emul)
cflags-y	+= -mabi=64
endif

undef-all += -UMIPSEB -U_MIPSEB -U__MIPSEB -U__MIPSEB__
undef-all += -UMIPSEL -U_MIPSEL -U__MIPSEL -U__MIPSEL__
predef-be += -DMIPSEB -D_MIPSEB -D__MIPSEB -D__MIPSEB__
predef-le += -DMIPSEL -D_MIPSEL -D__MIPSEL -D__MIPSEL__

ifdef CONFIG_CPU_BIG_ENDIAN
cflags-y += $(shell $(CC) -dumpmachine |grep -q 'mips.*el-.*' && echo -EB $(undef-all) $(predef-be))
ldflags-y += $(shell $(CC) -dumpmachine |grep -q 'mips.*el-.*' && echo -EB )
endif

ifdef CONFIG_CPU_LITTLE_ENDIAN
cflags-y += $(shell $(CC) -dumpmachine |grep -q 'mips.*el-.*' || echo -EL $(undef-all) $(predef-le))
ldflags-y += $(shell $(CC) -dumpmachine |grep -q 'mips.*el-.*' || echo -EL )
endif

LDFLAGS += $(ldflags-y) -m $(ld-emul)

#
# CPU-dependent compiler/assembler options for optimization.
#
cflags-$(CONFIG_CPU_MIPS32_R1) += $(call cc-option,-march=mips32,-mips32 -U_MIPS_ISA -D_MIPS_ISA=_MIPS_ISA_MIPS32) -Wa,-mips32 -Wa,--trap
cflags-$(CONFIG_CPU_MIPS32_R2) += $(call cc-option,-march=mips32r2,-mips32r2 -U_MIPS_ISA -D_MIPS_ISA=_MIPS_ISA_MIPS32) -Wa,-mips32r2 -Wa,--trap
cflags-$(CONFIG_CPU_MIPS64_R1) += $(call cc-option,-march=mips64,-mips64 -U_MIPS_ISA -D_MIPS_ISA=_MIPS_ISA_MIPS64) -Wa,-mips64 -Wa,--trap
cflags-$(CONFIG_CPU_MIPS64_R2) += $(call cc-option,-march=mips64r2,-mips64r2 -U_MIPS_ISA -D_MIPS_ISA=_MIPS_ISA_MIPS64) -Wa,-mips64r2 -Wa,--trap

CPPFLAGS += -DTEXT_BASE=$(CONFIG_TEXT_BASE)

ifndef CONFIG_MODULES
# Add cleanup flags
CPPFLAGS += -fdata-sections -ffunction-sections
LDFLAGS_barebox += -static --gc-sections
endif

LDFLAGS_barebox += -nostdlib

machine-$(CONFIG_MACH_MIPS_MALTA)	:= malta
board-$(CONFIG_BOARD_QEMU_MALTA)	:= qemu-malta

machine-$(CONFIG_MACH_MIPS_BCM47XX)	:= bcm47xx
board-$(CONFIG_BOARD_DLINK_DIR320)	:= dlink-dir-320

machine-$(CONFIG_MACH_MIPS_XBURST)	:= xburst
board-$(CONFIG_BOARD_RZX50)	:= rzx50

machdirs := $(patsubst %,arch/mips/mach-%/,$(machine-y))

ifeq ($(KBUILD_SRC),)
CPPFLAGS += $(patsubst %,-I%include,$(machdirs))
else
CPPFLAGS += $(patsubst %,-I$(srctree)/%include,$(machdirs))
endif

ifeq ($(incdir-y),)
incdir-y := $(machine-y)
endif
INCDIR   := arch-$(incdir-y)

ifneq ($(board-y),)
BOARD := arch/mips/boards/$(board-y)/
else
BOARD :=
endif

ifneq ($(machine-y),)
MACH  := arch/mips/mach-$(machine-y)/
else
MACH  :=
endif

CPPFLAGS += -I$(BOARD)/include

common-y += $(BOARD) $(MACH)
common-y += arch/mips/lib/
common-y += arch/mips/boot/

CPPFLAGS += $(cflags-y)
CFLAGS += $(cflags-y)

lds-$(CONFIG_GENERIC_LINKER_SCRIPT)   := arch/mips/lib/barebox.lds

CLEAN_FILES    += arch/mips/lib/barebox.lds barebox.map barebox.S
