from SConsdir.ConfigFile import ConfigFile

config = ConfigFile()
config.read(".config")

allflags = "-include config.h -include include/generated/autoconf.h -D__BAREBOX__ -Iinclude -Iarch/x86/include -D__SANDBOX__"
asflags = "%s -I. -D__ASSEMBLY__ -c" % allflags
cflags = "%s -fvisibility=default -fno-builtin -D__KERNEL__ -DCONFIG_MEMORY_LAYOUT_DEFAULT" % allflags
ldflags = "-nostdlib"

gcc_path = Dir("E:/vm-shared/i386-elf/bin")
asm = gcc_path.File("i386-elf-gcc.exe")
gcc = gcc_path.File("i386-elf-gcc.exe")
ld  = gcc_path.File("i386-elf-ld.exe")

env = Environment(
    AS = asm,
    ASFLAGS = asflags, 

    CC = gcc,
    CCFLAGS = cflags,
    
    LINK = ld,
    LINKFLAGS = ldflags)

srcs = []

SConscript(dirs=['common', 'fs', 'commands', 'lib'], exports='env srcs config')

bootfiles = Glob('arch/x86/boot/*.S')
bootfiles += ['arch/x86/lib/gdt.c']
bootfiles += Glob('arch/x86/boot/*.c')
bootfiles += ['arch/x86/mach-i386/reset.c']
drivers = Glob('drivers/base/*.c')
console = Glob('console/*.c')


env.Library('libbase', srcs+drivers+console)
env.Program('minibox', bootfiles, 
                       LIBS=['libbase'], LIBPATH='.',
                       LINKFLAGS="-T build/barebox.lds")
