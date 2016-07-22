del /F /Q /S BUILD\
del *.obj
dater /CREATEMAKEFILE
move dater.mak BUILD\dater.mak
AMISDL project.ldl
dir /B project\*.C			>> BUILD\src.lst
dir /B project\X64\*.asm	>> BUILD\asmsrc.lst
dir /B MiscLib\*.C			>> BUILD\src.lst
dir /B CpuLib\X64\*.asm		>> BUILD\asmsrc.lst
echo OBJS = >> BUILD\token.mak
FOR /F "tokens=1 delims=." %%I in (BUILD\src.lst) DO echo OBJS = $(OBJS) $(BUILD_DIR)\%%I.OBJ >> BUILD\token.mak
FOR /F "tokens=1 delims=." %%I in (BUILD\asmsrc.lst) DO echo ASMOBJS = $(ASMOBJS) $(BUILD_DIR)\%%I.OBJ >> BUILD\token.mak

nmake -f scripts\kernel64.mak all -nologo