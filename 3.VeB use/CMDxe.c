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
#include <Dxe.h>
#include <token.h>
#include <AmiLib.h>
#include <AmiDxeLib.h>
#include <setup.h>
#include <AMICSPLIBInc.H>

EFI_STATUS CMDXE_Init(IN EFI_HANDLE ImageHandle,
	IN EFI_SYSTEM_TABLE *SystemTable)
{
	/*
	EFI_GUID	gClareProGuid = EFI_PEI_READ_ONLY_VARIABLE_PPI_GUID;
	EFI_GUID	gSetupGuid = SETUP_GUID;
	SETUP_DATA	SetupData;
	UINTN	nVariableSize = sizeof(SETUP_DATA);

	InitAmiLib(ImageHandle,SystemTable);
	TEMPLATE_PROTOCOL *pClareProtocol = NULL
	Status = (EFI_BOOT_SERVICES).LocateProtocol(
				&gClareProGuid,
				NULL,
				&pClareProtocol);
	
	Status = pClareProtocol->GetVariable (
              L"Setup",
              &gSetupGuid,
              NULL,
              &nVariableSize,
              &SetupData
              );
	
	*/

	EFI_GUID	gSetupGuid = SETUP_GUID;
	SETUP_DATA	SetupData;
	UINTN	nVariableSize = sizeof(SETUP_DATA);
	EFI_STATUS Status;
	InitAmiLib(ImageHandle,SystemTable);
	Status = pRS->GetVariable (
              L"Setup",
              &gSetupGuid,
              NULL,
              &nVariableSize,
              &SetupData
              );
	if(EFI_ERROR(Status))
	{
		IoWrite16(0x80,0xA1A1);
		while(1);
	}
	//MMIO_WRITE8(0x80,0x01);
	//pBS->Stall(6000000);
	IoWrite16(0x80, 0x0b0b);
	pBS->Stall(1000000);
	IoWrite16(0x80, SetupData.waitForF1);
	pBS->Stall(3000000);
	IoWrite16(0x80, 0x0b0b);
	pBS->Stall(1000000);
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