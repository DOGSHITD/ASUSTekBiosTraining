//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

//**********************************************************************
// $Header: /Alaska/Tools/template.c 6     1/13/10 2:13p Felixp $
//
// $Revision: 6 $
//
// $Date: 1/13/10 2:13p $
//**********************************************************************
// Revision History
// ----------------
// $Log: /Alaska/Tools/template.c $
// 
// 6     1/13/10 2:13p Felixp
// 
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:  <This File's Name>
//
// Description:	
//
//<AMI_FHDR_END>
//**********************************************************************

#include <Efi.h>
#include <Pei.h>
#include <token.h>
#include <AmiLib.h>
#include <AmiPeiLib.h>
#include <AmiCspLib.h>
#include <Setup.h>
#include <PPI\STALL.h>
#include <Ppi\ReadOnlyVariable.h>
#include <Core\PeiDebug.h>


#define CLARE_PPI_GUID \
{0x75d27452, 0x61c, 0x4130, 0xa9, 0x18, 0x66, 0x11, 0xbb, 0x81, 0xf4, 0x95}

EFI_STATUS CMPEI_Init(IN EFI_FFS_FILE_HEADER *FfsHeader,IN EFI_PEI_SERVICES **PeiServices )
{
	//Read Data
	EFI_GUID	gClarePpiGuid = EFI_PEI_READ_ONLY_VARIABLE_PPI_GUID;
	EFI_GUID	gSetupGuid = SETUP_GUID;
	EFI_GUID gClareStallGuid=EFI_PEI_STALL_PPI_GUID;
	EFI_PEI_STALL_PPI	*StallPpi;
	EFI_PEI_READ_ONLY_VARIABLE_PPI *XiaoDongPpi;
	SETUP_DATA	SetupData;
	UINTN	nVariableSize = sizeof(SETUP_DATA);
	
	EFI_STATUS Status =  (**PeiServices).LocatePpi (
		    		PeiServices,
		    		&gClarePpiGuid, 
		    		0,
					NULL, 
					&XiaoDongPpi              // PPI
    );
	if(EFI_ERROR(Status))
	{
		IoWrite16(0x80,0x0101);
		while(1);
	}
	Status = XiaoDongPpi->GetVariable(
		PeiServices,
		L"Setup",
		&gSetupGuid,
		NULL,
		&nVariableSize,
		(VOID *)&SetupData);
	if(EFI_ERROR(Status))
	{
		IoWrite16(0x80,0x0202);
		while(1);
	}


	Status = (**PeiServices).LocatePpi (
	PeiServices,
	&gClareStallGuid,
	0,
	NULL,
	&StallPpi
	);
	if(EFI_ERROR(Status))
	{
		IoWrite16(0x80,0x0303);
		while(1);
	}
	IoWrite16(0x80, 0x0a0a);
	StallPpi->Stall(PeiServices, StallPpi, 1000000);
	IoWrite16(0x80, SetupData.waitForF1);
	StallPpi->Stall(PeiServices, StallPpi, 3000000);
	IoWrite16(0x80, 0x0a0a);
	StallPpi->Stall(PeiServices, StallPpi, 1000000);
	return EFI_SUCCESS;
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************