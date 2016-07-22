// Module specific Includes
#include <Efi.h>
#include <Pei.h>
#include <token.h>
#include <AmiPeiLib.h>
#include <Setup.h>
#include <ppi\smbus.h>
#include <ppi\readonlyvariable.h>
#include <ppi\NBPPI.h>
#include <PPI\STALL.h>
#include "Board\SB\SB.h"
#define  DataPort 0x60
#define  ComdPort 0x64
void InitStall(EFI_PEI_SERVICES **PeiServices, EFI_PEI_STALL_PPI *StallPpi)
{
	EFI_GUID gClareStallGuid=EFI_PEI_STALL_PPI_GUID;
	//delay
	EFI_STATUS Status = (**PeiServices).LocatePpi (
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
}
void WaitIBE()
{
	unsigned char IBF=0xff;
	while(IBF & 0x02)
		IBF=IoRead8(ComdPort);
}
void WaitIBE_Data()
{
	unsigned char IBF=0xff;
	do{
		IBF=IoRead8(ComdPort);
	}while((IBF&0x0a)!=0x00);
}
void WaitOBF()
{
	unsigned char OBF=0x00;
	while(!(OBF & 0x01))
		OBF=IoRead8(ComdPort);
}
void InitKeyboard(EFI_PEI_SERVICES **PeiServices,EFI_PEI_STALL_PPI *StallPpi)
{
	unsigned char rlt;
	WaitIBE();
	IoWrite8(ComdPort,0xaa);//KBC Self-Test
	WaitOBF();
	rlt=IoRead8(DataPort);
	if(rlt!=0x55)
	{
		IoWrite16(0x80, 0xE001);
	    StallPpi->Stall(PeiServices, StallPpi, 3000000);
		return;
	}
	
	WaitIBE();
	IoWrite8(ComdPort,0xab);//Interface Test
	WaitOBF();
	rlt=IoRead8(DataPort);
	if(rlt!=0x00)
	{
		IoWrite16(0x80, 0xE002);
	    StallPpi->Stall(PeiServices, StallPpi, 3000000);
		return;
	}

	/*
	WaitIBE_Data();
	IoWrite8(DataPort,0xF0);//command
	WaitIBE();
	IoWrite8(DataPort,0x01);
	*/

	WaitIBE();
	IoWrite8(ComdPort,0x60);//command
	WaitIBE();
	IoWrite8(DataPort,0x08);

	WaitIBE_Data();
	IoWrite8(DataPort,0xff);//Keyboard Reset
	WaitOBF();
	rlt=IoRead8(DataPort);   	//Read AF
	IoWrite8(0x80, rlt);
	StallPpi->Stall(PeiServices, StallPpi, 1000000);

	WaitIBE();
	IoWrite8(ComdPort,0xae);//Enable KB Interface
	WaitOBF();
	rlt=IoRead8(DataPort);		//Read AA
	IoWrite8(0x80, rlt);
	StallPpi->Stall(PeiServices, StallPpi, 1000000);
}
VOID TDC_Training_2015_KB_HomeWork(EFI_PEI_SERVICES **PeiServices,EFI_PEI_STALL_PPI *StallPpi)
{
/***************************************
      Add your homework in here.
****************************************/
	unsigned char scanCode,statusRegister=0;
	UINT16 printCode=0x0000;
	IoWrite16(0x80, 0x0a0a);
	StallPpi->Stall(PeiServices, StallPpi, 1000000);
	InitKeyboard(PeiServices,StallPpi);
	IoWrite16(0x80, 0x0c0c);
	StallPpi->Stall(PeiServices, StallPpi, 1000000);
	//begin to read the scan code
	while(1)
	{
		statusRegister=IoRead8(ComdPort);		
		if((statusRegister & 0x80) == 0x80)
		{
			IoWrite16(0x80, 0x9e01);
			StallPpi->Stall(PeiServices, StallPpi, 2000000);
			break;
		}
		else if((statusRegister & 0x01) == 0x01)
		{
			scanCode=IoRead8(DataPort);
			printCode=(printCode<<4) | (scanCode & 0x0f );
			StallPpi->Stall(PeiServices, StallPpi, 1000000);
			printCode=(printCode<<4) | ((scanCode & 0xf0)>>4);
			IoWrite16(0x80,printCode);
			if(scanCode==0x76)
				break;
			StallPpi->Stall(PeiServices, StallPpi, 1000000);
		}
	}
	StallPpi->Stall(PeiServices, StallPpi, 2000000);

/***************************************/
}


EFI_STATUS ProjectPEIEntry (
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN EFI_PEI_SERVICES **PeiServices )
{
	EFI_PEI_STALL_PPI *StallPpi;
	EFI_GUID gClareStallGuid=EFI_PEI_STALL_PPI_GUID;
	//delay
	EFI_STATUS Status = (**PeiServices).LocatePpi (
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
	//InitStall(PeiServices,*StallPpi);	
	TDC_Training_2015_KB_HomeWork(PeiServices,StallPpi);

	return EFI_SUCCESS;
}

