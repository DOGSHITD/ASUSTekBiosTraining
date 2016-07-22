!INCLUDE "BUILD\token.mak"

#===============================================================================================
# General Definition
#===============================================================================================
CFLAGS= /D___STDC___ /nologo /W3 /Gm /Zi /Od /GF /Gy /Gs8192 /QIfist /X /DEFI32 /DEFI_DEBUG /DEFI_APP_110
LFLAGS= /NOLOGO /MACHINE:X86 /SUBSYSTEM:CONSOLE /NODEFAULTLIB /IGNORE:4086 /DEBUG /OPT:REF
AFLAGS= /nologo /W3 /WX /c /Zi /DDEBUG /DEFI32
PROCESSOR= ia32

ROOT_DIR = $(MAKEDIR)
BUILD_DIR = $(ROOT_DIR)\Build
MISC_LIB_DIR = $(ROOT_DIR)\MiscLib
CPU_LIB_DIR = $(ROOT_DIR)\CpuLib
EFI_TOOLKIT_DIR = $(ROOT_DIR)\EfiToolKit_20
TOOLS_DIR = $(EFI_TOOLKIT_DIR)\BIN
BIN_DIR = $(ROOT_DIR)\BIN

TARGET=app
MAKE= $(TOOLS_DIR)\VC7.1\nmake.exe
CC= $(TOOLS_DIR)\VC7.1\cl.exe
ASM= $(TOOLS_DIR)\VC7.1\ml.exe
LINK= $(TOOLS_DIR)\VC7.1\link.exe
LIB= $(TOOLS_DIR)\VC7.1\lib.exe
OUTPUT_FILE=$(BIN_DIR)\$(PROJECT_NAME).efi
CPU_LIB_SRC_DIR = $(ROOT_DIR)\CpuLib\IA32
FWIMAGE= $(TOOLS_DIR)\fwimage.exe


INCLUDEBASE= include\efi110
INCLUDESHELLBASE= include\efishell
INCLUDE= -I $(EFI_TOOLKIT_DIR)\$(INCLUDEBASE)
INCLUDE= -I $(EFI_TOOLKIT_DIR)\$(INCLUDEBASE)\protocol $(INCLUDE)
INCLUDE= -I $(EFI_TOOLKIT_DIR)\$(INCLUDEBASE)\$(PROCESSOR) $(INCLUDE)
INCLUDE= -I $(EFI_TOOLKIT_DIR)\$(INCLUDESHELLBASE) $(INCLUDE)
INCLUDE= -I $(PROJECT_DIR) $(INCLUDE)
INCLUDE= -I $(MISC_LIB_DIR) $(INCLUDE)
INCLUDE= -I $(CPU_LIB_DIR) $(INCLUDE)
LIBALL= $(EFI_TOOLKIT_DIR)\Lib\LibEfiAll.lib
IMAGE_ENTRY_POINT=$(IMAGE_ENTRY_POINT)


#===============================================================================================
# General Dependency
#===============================================================================================
all: $(OUTPUT_FILE)

#===============================================================================================
# Build $(PROJECT_NAME).efi
#===============================================================================================
$(OUTPUT_FILE) : $(BUILD_DIR)\$(PROJECT_NAME).dll
	$(FWIMAGE) $(TARGET) $(BUILD_DIR)\$(PROJECT_NAME).dll $(OUTPUT_FILE)
	
#===============================================================================================
# Build $(PROJECT_NAME).dll
#===============================================================================================
$(BUILD_DIR)\$(PROJECT_NAME).dll: $(OBJS) $(ASMOBJS) $(LIBALL)
	$(LINK) $(LFLAGS) /NODEFAULTLIB /DLL /ENTRY:$(IMAGE_ENTRY_POINT) $(LIBALL) $(OBJS) $(ASMOBJS) /OUT:$(BUILD_DIR)\$(PROJECT_NAME).dll

#===============================================================================================
# Build OBJS
#===============================================================================================
$(OBJS) :
{$(PROJECT_DIR)}.c{$(BUILD_DIR)}.obj:	
	$(CC) $(CFLAGS) $(INCLUDE) -c $< /Fo$@

$(OBJS) :
{$(MISC_LIB_DIR)}.c{$(BUILD_DIR)}.obj:	
	$(CC) $(CFLAGS) $(INCLUDE) -c $< /Fo$@

$(OBJS) :
{$(CPU_LIB_SRC_DIR)}.c{$(BUILD_DIR)}.obj:	
	$(CC) $(CFLAGS) $(INCLUDE) -c $< /Fo$@

#===============================================================================================
# Build ASMOBJS
#===============================================================================================
!IFDEF ASMOBJS
$(ASMOBJS) :
{$(PROJECT_DIR)\IA32}.asm{$(BUILD_DIR)}.obj:	
	$(ASM) $(AFLAGS) /c /coff $<
	move *.obj $@
!ENDIF