@REM REM ----------------- host tools
@REM set HOST_GCC=gcc
@REM 
@REM %HOST_GCC% tools\kallsyms.c -o build\kallsyms.exe

REM ----------------- target compilation

set GCC_PATH=E:\vm-shared\i386-elf\bin
set CPP=%GCC_PATH%\i386-elf-cpp.exe
set AS=%GCC_PATH%\i386-elf-gcc.exe
set GCC=%GCC_PATH%\i386-elf-gcc.exe
set LD=%GCC_PATH%\i386-elf-ld.exe
set AR=%GCC_PATH%\i386-elf-ar.exe
set RANLIB=%GCC_PATH%\i386-elf-ranlib.exe
set OBJCOPY=%GCC_PATH%\i386-elf-objcopy.exe

set ALLFLAGS=-include config.h -include include\generated\autoconf.h -D__BAREBOX__ -Iinclude -Iarch\x86\include
set ASFLAGS=%ALLFLAGS% -I. -D__ASSEMBLY__
set CFLAGS=%ALLFLAGS% -fvisibility=default -fno-builtin -D__KERNEL__ -DCONFIG_MEMORY_LAYOUT_DEFAULT
set LDFLAGS=-nostdlib

REM ----------------- generate compile.h
python scripts\mkcompile_h.py include\generated\compile.h sandbox gcc

REM ----------------- base library compilation

mkdir build\base
del /q build\base\*
del /q build\libbase.a

for %%F in (common\*.c) do %GCC% %CFLAGS% %%F -c -o build\base\common_%%~nF.o
for %%F in (lib\*.c) do %GCC% %CFLAGS% %%F -c -o build\base\base_%%~nF.o
for %%F in (fs\*.c) do %GCC% %CFLAGS% %%F -c -o build\base\fs_%%~nF.o
for %%F in (drivers\base\*.c) do %GCC% %CFLAGS% %%F -c -o build\base\drivers_%%~nF.o
for %%F in (console\*.c) do %GCC% %CFLAGS% %%F -c -o build\base\console_%%~nF.o

REM ----------------- Arch specific files

for %%F in (x86\*.c) do %GCC% %CFLAGS% %%F -c -o build\base\x86_%%~nF.o

%AR% crs build\libbase.a build\base\*.o

%RANLIB% build\libbase.a

REM ----------------- minibox compilation

%AS% %ASFLAGS% arch\x86\boot\boot_main.S -c -o build\boot_main.o
%AS% %ASFLAGS% arch\x86\boot\boot_hdisk.S -c -o build\boot_hdisk.o
%AS% %ASFLAGS% arch\x86\boot\bioscall.S -c -o build\bioscall.o
%AS% %ASFLAGS% arch\x86\boot\pmjump.S -c -o build\pmjump.o

%GCC% %CFLAGS% arch\x86\lib\gdt.c -c -o build\gdt.o
%GCC% %CFLAGS% arch\x86\boot\main_entry.c -c -o build\main_entry.o
%GCC% %CFLAGS% arch\x86\boot\tty.c -c -o build\tty.o
%GCC% %CFLAGS% arch\x86\boot\prepare_uboot.c -c -o build\prepare_uboot.o
%GCC% %CFLAGS% arch\x86\boot\regs.c -c -o build\regs.o
%GCC% %CFLAGS% arch\x86\boot\a20.c -c -o build\a20.o

%GCC% %CFLAGS% arch\x86\mach-i386\reset.c -c -o build\reset.o

del /q build\barebox.lds

%CPP% -P arch\x86\lib\barebox.lds.S -I. -include include\generated\autoconf.h -Iinclude -Iarch\x86\mach-i386\include -DTEXT_BASE=0x00007c00 -DCONFIG_X86 -o build\barebox.lds

%LD% %LDFLAGS% build\main_entry.o build\boot_main.o build\boot_hdisk.o build\prepare_uboot.o build\reset.o build\tty.o build\bioscall.o build\pmjump.o build\gdt.o build\regs.o build\a20.o build\libbase.a -T build\barebox.lds -o build\minibox

%OBJCOPY% -I elf32-i386 -O binary -j .bootsector build\minibox build\minibox_mbr 
%OBJCOPY% -I elf32-i386 -O binary build\minibox build\minibox.img
