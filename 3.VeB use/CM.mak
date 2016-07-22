#**********************************************************************
#**********************************************************************
#**                                                                  **
#**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
#**                                                                  **
#**                       All Rights Reserved.                       **
#**                                                                  **
#**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
#**                                                                  **
#**                       Phone: (770)-246-8600                      **
#**                                                                  **
#**********************************************************************
#**********************************************************************

#**********************************************************************
# $Header: /Alaska/Tools/template.mak 6     1/13/10 2:13p Felixp $
#
# $Revision: 6 $
#
# $Date: 1/13/10 2:13p $
#**********************************************************************
# Revision History
# ----------------
# $Log: /Alaska/Tools/template.mak $
# 
# 6     1/13/10 2:13p Felixp
# 
#**********************************************************************
#<AMI_FHDR_START>
#
# Name:	<ComponentName>.mak
#
# Description:	
#
#<AMI_FHDR_END>
#**********************************************************************

all : CMPEI CMDXE

BUILD_CM_DIR = $(BUILD_DIR)\$(CM_DIR)


$(BUILD_DIR)\CM.mak : $(CM_DIR)\CM.cif $(BUILD_RULES)
	$(CIF2MAK) $(CM_DIR)\CM.cif $(CIF2MAK_DEFAULTS)

CMPEI : $(BUILD_DIR)\CM.mak CMPEIBin
CMPEI_OBJECTS = $(BUILD_CM_DIR)\CMPEI.obj
CMPEIBin:  $(AMIPEILIB) $(AMICSPLib)
    $(MAKE) /$(MAKEFLAGS) $(BUILD_DEFAULTS)\
        /f $(BUILD_DIR)\CM.mak all\
		NAME=CMPEI \
		MAKEFILE=$(BUILD_DIR)\CM.mak \
		OBJECTS="$(CMPEI_OBJECTS)" \
		GUID=3B193199-6F8E-4174-8ED4-8CE62B8D1269 \
        ENTRY_POINT=CMPEI_Init \
        TYPE=PEIM \
		DEPEX1=$(CM_DIR)\CMPEI.DXS DEPEX1_TYPE=EFI_SECTION_PEI_DEPEX \
		COMPRESS=0

CMDXE:$(BUILD_DIR)\CM.mak CMDXEBin
CMDXE_OBJECTS = $(BUILD_CM_DIR)\CMDXE.obj
CMDXEBin : $(AMICSPLib) $(AMIDXELIB)
	$(MAKE) /$(MAKEFLAGS) $(BUILD_DEFAULTS)\
		/f $(BUILD_DIR)\CM.mak all\
		NAME=CMDXE\
		MAKEFILE=$(BUILD_DIR)\CM.mak \
		OBJECTS="$(CMDXE_OBJECTS)" \
		GUID=0111ADAC-735B-42cf-9340-BD8CB875665F \
		ENTRY_POINT=CMDXE_Init\
		TYPE=BS_DRIVER \
		DEPEX1=$(CM_DIR)\CMDXE.DXS DEPEX1_TYPE=EFI_SECTION_DXE_DEPEX \
		COMPRESS=1
		
SetupSdbs : $(BUILD_DIR)\CM.mak CZGSDB
SetupBin : $(BUILD_DIR)\CMsetup.obj

CZGSDB : 
	$(MAKE) /$(MAKEFLAGS) $(BUILD_DEFAULTS)\
		/f $(BUILD_DIR)\CM.mak all\
		TYPE=SDB NAME=CM STRING_CONSUMERS=$(CM_DIR)\CM.sd

$(BUILD_DIR)\CMsetup.obj : $(CM_DIR)\CMsetup.c $(BUILD_DIR)\SetupStrTokens.h
    $(CC) $(CFLAGS) /Fo$(BUILD_DIR)\ $(CM_DIR)\CMsetup.c



#**********************************************************************
#**********************************************************************
#**                                                                  **
#**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
#**                                                                  **
#**                       All Rights Reserved.                       **
#**                                                                  **
#**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
#**                                                                  **
#**                       Phone: (770)-246-8600                      **
#**                                                                  **
#**********************************************************************
#**********************************************************************