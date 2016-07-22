#include <efi.h>
#include <shell.h>
#include <CpuLib.h>
#define BIT(i)     ( 0x01<<i )
#define PCI_CONFIG_ADDR_PORT 0x0CF8
#define PCI_CONFIG_DATA_PORT 0x0CFC
//**********************************************************************
//atoh
//**********************************************************************
EFI_STATUS atoh(UINT8 c, UINT8 *h)
{
	if((c>='a')&&(c<='f'))
	{
		*h = c-0x61+0x0a;
		return EFI_SUCCESS;
	}
	if((c>='A')&&(c<='F'))
	{
		*h = c-0x41+0x0a;
		return EFI_SUCCESS;
	}
	if((c>='0')&&(c<='9'))
	{
		*h = c-0x30;
		return EFI_SUCCESS;
	}

	return EFI_UNSUPPORTED;
}

//**********************************************************************
//StringToHex
//**********************************************************************
EFI_STATUS StringToHex(UINT16 *string, UINT32 *Data)
{

	UINT8		s;
	UINT8		i,j;
	EFI_STATUS	Status = EFI_SUCCESS;
	UINT32		Result = 0;

//	Print(L"StringToHex %s\n", string);

	for(i=0;i<8,*string!=0;i++)
	{
		s = (UINT8)*string;
		if(EFI_ERROR(atoh(s, &j)))
		{
			Status = EFI_UNSUPPORTED;
			break;
		}
		Result = Result*16;
		Result +=  j;
		++string;
	}

	*Data = Result;

	return(Status);
}
//====================================================================================
// PciScan_By_Protocol
//====================================================================================
EFI_STATUS PciScan_By_Protocol()
{
	EFI_STATUS	Status;
	UINTN 		HandleCount;
	EFI_HANDLE 	*HandleBuffer;
	UINTN 		HandleIndex;
	EFI_PCI_IO_PROTOCOL     *PciIo;
	UINTN		Seg, Bus, Dev, Fun;
	UINT32		dwDidVid;
	EFI_GUID	EfiPciIOProtocolGuid = EFI_PCI_IO_PROTOCOL_GUID;

	Status = BS->LocateHandleBuffer (
			ByProtocol,
			&EfiPciIOProtocolGuid,
			NULL,
			&HandleCount,
			&HandleBuffer
	);

	if (EFI_ERROR (Status)) {
		Print(L"LocateHandleBuffer Faile, Status=%r\n", Status);
		return EFI_UNSUPPORTED;
	}

	Print(L"==========================================================\n");
	Print(L"Handle   Seg      Bus      Dev      Fun      DIDVID\n");
	Print(L"===========================================================\n");
	for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
		Status = BS->HandleProtocol (
				HandleBuffer[HandleIndex],
				&EfiPciIOProtocolGuid,
				&PciIo
				);
		PciIo->GetLocation(PciIo,&Seg,&Bus,&Dev,&Fun);
		PciIo->Pci.Read(PciIo, EfiPciIoWidthUint32, 0x00, 1, &dwDidVid);
		Print(L"%08X %02X %02X %02X %02X %04X\n", HandleBuffer[HandleIndex], Seg, Bus, Dev, Fun, dwDidVid);
	}
}

//**********************************************************************
//ReadPCIDWord
//**********************************************************************
UINT32 ReadPCIDword(UINT8 bus, UINT8 dev, UINT8 func, UINT8 reg)
{
	UINT32 Data32;
	Data32 = 0x80000000 + (bus<<16) + (dev<<11) + (func<<8) + reg;
	IoWrite32(0xCF8, Data32);
	Data32 = IoRead32(0xCFC);
	return(Data32);
}
//====================================================================================
// PciDump_By_Io
//====================================================================================
EFI_STATUS PciDump_By_Io(UINT8 Bus, UINT8 Dev, UINT8 Func)
{
	UINT32	i, j;

	Print(L"     x00000000 x00000004 x00000008 x0000000C\n");
	for(i=0;i<=0x0F;i++)
	{
		Print(L"0x%02x ",i*0x10);
		for(j=0;j<=0x03;j++)
		{
			Print(L" %08x ", ReadPCIDword(Bus, Dev, Func,((i<<4)+(j<<2))));
		}
		Print(L"\n");
	}
}
//====================================================================================
// ShowHelpMessage
//====================================================================================
VOID ShowHelpMessage()
{
	Print(L"PCIDUMP                          : Dump All PCI\n");
	Print(L"PCIDUMP [ Bus ] [ Dev ] [ Func ] : Dump Specify PCI\n");
}
//====================================================================================
//ApplicationEntryPoint
//
// Notes:
//       General Definitions After InitializeShellApplication:
//         EFI_SHELL_INTERFACE SI
//         ST  = SystemTable;
//         BS  = SystemTable->BootServices;
//         RT  = SystemTable->RuntimeServices;          
//====================================================================================
EFI_STATUS ApplicationEntryPoint(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	EFI_STATUS	Status = EFI_SUCCESS;
	UINT32		Bus, Dev, Func;

	InitializeShellApplication (ImageHandle, SystemTable);

	switch(SI->Argc){
		case 1:
			PciScan_By_Protocol();
			break;
		case 2:
			if((wcscmp(SI->Argv[1], L"/?")==0) || (wcscmp(SI->Argv[1], L"-?")==0) || (wcscmp(SI->Argv[1], L"/H")==0) || (wcscmp(SI->Argv[1], L"/h")==0)){
				ShowHelpMessage();
			}
			break;
		case 4:
			Status = StringToHex(SI->Argv[1], &Bus);
			if(EFI_ERROR(Status) || (Bus>0xFF)){
				Print(L"Invalid Bus Number\n");
				return EFI_UNSUPPORTED;
			}
			Status = StringToHex(SI->Argv[2], &Dev);
			if(EFI_ERROR(Status) || (Dev>0x1F)){
				Print(L"Invalid Bus Number\n");
				return EFI_UNSUPPORTED;
			}
			Status = StringToHex(SI->Argv[3], &Func);
			if(EFI_ERROR(Status) || (Bus>0x07)){
				Print(L"Invalid Bus Number\n");
				return EFI_UNSUPPORTED;
			}
			PciDump_By_Io((UINT8)Bus, (UINT8)Dev, (UINT8)Func);
			break;
		default:
			ShowHelpMessage();
			break;
	}

	return Status; 
}
//====================================================================================
EFI_STATUS GetSystemMemoryMap(
    OUT EFI_MEMORY_DESCRIPTOR **MemMap,
    OUT UINTN *MemDescSize,
    OUT UINTN *MemEntriesCount
)
{
    EFI_STATUS Status;
    UINTN MemMapSize, MemMapKey;
    UINT32 MemDescVer;
    *MemMap = NULL;

    MemMapSize = 0; // GetMemoryMap will return the size needed for the map
    Status = BS->GetMemoryMap(&MemMapSize, *MemMap,
                    &MemMapKey, MemDescSize, &MemDescVer);

    // The following memory allocation may alter memory map.
    // Let's add space for 5 more descriptors to be sure buffer is big enough
    if (Status != EFI_BUFFER_TOO_SMALL) return Status;
    MemMapSize += 5 * *MemDescSize;
    Status = BS->AllocatePool(EfiBootServicesData, MemMapSize, MemMap);
    if(EFI_ERROR(Status))
	{
		Print(L"0002 Some error happened!!!\n");
	}

    Status = BS->GetMemoryMap(&MemMapSize, *MemMap,
                    &MemMapKey, MemDescSize, &MemDescVer);
    if(EFI_ERROR(Status))
	{
		Print(L"0002 Some error happened!!!\n");
	}

    *MemEntriesCount = (UINT16)(MemMapSize / *MemDescSize);
    return Status;
}

void print8Char(UINT32 mAddr)
{
	Print(L"%c%c%c%c",MemRead8(mAddr),MemRead8(mAddr+1),MemRead8(mAddr+2),MemRead8(mAddr+3));
	Print(L"%c%c%c%c is ",MemRead8(mAddr+4),MemRead8(mAddr+5),MemRead8(mAddr+6),MemRead8(mAddr+7));
	Print(L"%08llx\n",mAddr);
}
void print4Char(UINT32 mAddr)
{
	Print(L"%c%c%c%c is ",MemRead8(mAddr),MemRead8(mAddr+1),MemRead8(mAddr+2),MemRead8(mAddr+3));
	Print(L"%08llx\n",mAddr);
}
UINT8 checkSum(UINT32 addr)
{
	UINT32 sum=0,i;
	for(i=0;i<20;i++)
	{
		sum += MemRead8(addr + i);
	}
	sum = sum & 0x0FF;
	if(sum == 0x0)
		return 0;
	return 1;
}
UINT32 ReadPMBase()
{
	UINT32 busNum,devNum,funNum;
	UINT32 baseCmd=0x80000000,cmd;
	UINT32 result=0;
	busNum=0;
	devNum=31;
	funNum=0;
	cmd = baseCmd|(busNum<<16)|(devNum<<11)|(funNum<<8);
	cmd = cmd + 0x40;
	IoWrite32(PCI_CONFIG_ADDR_PORT,cmd);
	result=IoRead32(PCI_CONFIG_DATA_PORT);
	Print(L"ACPI Base Address is 0x%08llx\n",result);
	return result;
}
UINT8 getFADTSum(UINT32 FADTAddr)
{
	UINT32 len,i,sum=0;

	len=MemRead32(FADTAddr + 4);
	for(i=0;i<len;i++)
	{
		if(i==9)
			continue;
		sum += MemRead8(FADTAddr+i);
	}
	sum = sum & 0x0ff;
	return (UINT8)sum;	
}
UINT8 getFADTTotalSum(UINT32 FADTAddr)
{
	UINT32 len,i,sum=0;
	len=MemRead32(FADTAddr + 4);
	Print(L"len is %d\n",len);
	for(i=0;i<len;i++)
	{
		sum += MemRead8(FADTAddr+i);
	}
	
	sum = sum & 0x0ff;
	return (UINT8)sum;	
}
EFI_STATUS ACPIEntryPoint(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	UINT32 bAddr=0xE0000,eAddr=0xFFFFE;
	UINT32 pLow,pHigh,i;
	UINT32 RSDP=0,XsdtAddress,RsdtAddress;
	UINT32 XsdtLen,EntryCount=0;
	UINT32 EntryAddr,EntryTable,FACPAddr=0,FACS,DSDT;
	UINT8 IAPC_BOOT_ARCH=0;
	UINT32 PMBase=0;
	UINT8 resetCode=0;
	UINT32 resetAddr;
	UINT8 FADTSum;
	InitializeShellApplication (ImageHandle, SystemTable);
	//Print(L"--------------------------------------------\n");


	while(bAddr < eAddr)
	{
		pLow=bAddr;
		pHigh=(bAddr+4);
		if(MemRead32(pLow) == 0x20445352 && MemRead32(pHigh) == 0x20525450)
		{
			if(!checkSum(bAddr))
			{
				//printf("find one RSD PTR maybe\n");
				RSDP=bAddr;
				print8Char(RSDP);
			}
		}
		bAddr+=2;
	}
	if(RSDP == 0)
	{
		Print(L"can not find a RDSP");
		return 1;
	}

	RsdtAddress=MemRead32(RSDP + 16);
	XsdtAddress=MemRead32(RSDP + 24);
	print4Char(RsdtAddress);
	print4Char(XsdtAddress);
	XsdtLen=MemRead32(XsdtAddress + 4);
	EntryCount=XsdtLen-36;		//sub other table size
	//printf("EntryCount is %d\n",EntryCount);
	for(i=0;i<EntryCount/8;i++)
	{
		EntryAddr=MemRead32(XsdtAddress + 36 + i*8);
		print4Char(EntryAddr);
		if(MemRead32(EntryAddr) == 0x50434146)
		{
			FACPAddr=EntryAddr;
		}
	}
	if(FACPAddr != 0)
	{
		FACS=MemRead32(FACPAddr + 36);
		print4Char(FACS);
		DSDT=MemRead32(FACPAddr + 40);
		print4Char(DSDT);
	}
	IAPC_BOOT_ARCH = MemRead8(FACPAddr + 109);
	Print(L"IAPC_BOOT_ARCH is %x\n",IAPC_BOOT_ARCH);
	IAPC_BOOT_ARCH = IAPC_BOOT_ARCH | BIT(4);	 //| 0x3F;//
	MemWrite8(FACPAddr + 109,IAPC_BOOT_ARCH);

	//PMBase=ReadPMBase();
	PMBase=MemRead32(FACPAddr + 64);
	Print(L"PMBase is %08llx\n",PMBase);

	resetCode = MemRead8(PMBase+0x01);
	resetCode = (resetCode & 0xC3) | 0x34;

	MemWrite32(FACPAddr + 0x78,PMBase+0x01);
	MemWrite8(FACPAddr + 0x80,resetCode);

	FADTSum=getFADTSum(FACPAddr);
	Print(L"S1 is %x\n",FADTSum);
	FADTSum = ~FADTSum;
	FADTSum = FADTSum+1;	
	Print(L"S2 is %x\n",FADTSum);
	MemWrite8(FACPAddr+9,FADTSum);

	FADTSum=getFADTTotalSum(FACPAddr);
	Print(L"S3 is %x\n",FADTSum);
	return EFI_SUCCESS; 
}

