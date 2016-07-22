del /F /Q /S BUILD\
dater /CREATEMAKEFILE
move dater.mak BUILD\dater.mak
AMISDL project.ldl
dir /B project\*.C			>> BUILD\src.lst
dir /B project\IA32\*.asm	>> BUILD\asmsrc.lst
dir /B MiscLib\*.C			>> BUILD\src.lst
dir /B CpuLib\IA32\*.asm	>> BUILD\asmsrc.lst
dir /B CpuLib\IA32\*.C		>> BUILD\src.lst
echo OBJS = >> BUILD\token.mak
FOR /F "tokens=1 delims=." %%I in (BUILD\src.lst) DO echo OBJS = $(OBJS) $(BUILD_DIR)\%%I.OBJ >> BUILD\token.mak
FOR /F "tokens=1 delims=." %%I in (BUILD\asmsrc.lst) DO echo ASMOBJS = $(ASMOBJS) $(BUILD_DIR)\%%I.OBJ >> BUILD\token.mak

nmake -f scripts\kernel32.mak all -nologo
