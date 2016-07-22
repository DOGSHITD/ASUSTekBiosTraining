#include <efi.h>
#include <shell.h>
#include <CpuLib.h>
#include <efiapi.h>
//#define cDebug 0
//#define cDebug 0
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
	#ifdef cDebug
	Print(L"000 MemoryMapSize is %08lx\n",MemMapSize);
	Print(L"000 MemMap is %08lx\n",MemMap);
	#endif
    // The following memory allocation may alter memory map.
    // Let's add space for 5 more descriptors to be sure buffer is big enough
    if (Status != EFI_BUFFER_TOO_SMALL) return Status;
    MemMapSize += 5 * *MemDescSize;
    Status = BS->AllocatePool(EfiBootServicesData, MemMapSize, MemMap);
    if(EFI_ERROR(Status))
	{
		Print(L"0002 Some error happened!!!\n");
	}
	#ifdef cDebug
	Print(L"001 MemoryMapSize is %08lx\n",MemMapSize);
	Print(L"001 MemMap is %08lx\n",MemMap);
	#endif
    Status = BS->GetMemoryMap(&MemMapSize, *MemMap,
                    &MemMapKey, MemDescSize, &MemDescVer);
	#ifdef cDebug
	Print(L"002 MemoryMapSize is %08lx\n",MemMapSize);
	Print(L"002 MemMap is %08lx\n",MemMap);
	#endif
    if(EFI_ERROR(Status))
	{
		Print(L"0002 Some error happened!!!\n");
	}

    *MemEntriesCount = (UINT16)(MemMapSize / *MemDescSize);
    return Status;
}
EFI_STATUS ClareEntryPoint(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	UINTN MemDescSize;
	EFI_MEMORY_DESCRIPTOR *MemMap;
	EFI_MEMORY_DESCRIPTOR *mm;
	UINTN count = 0;
	UINTN MemEntriesCount;
	UINT64 addrEnd=0;
	UINT32 PageCnt[14]={0},TotalPage=0;
	EFI_STATUS Status;
	InitializeShellApplication (ImageHandle, SystemTable);
	Print(L"************************   Clare mode   *******************************\n");
	Status = GetSystemMemoryMap(&MemMap, &MemDescSize, &MemEntriesCount);
	#ifdef cDebug
	return EFI_SUCCESS;
	#endif 
	if(EFI_ERROR(Status))
	{
		Print(L"Some error happened!!!\n");
	}
	else
	{
		Print(L"Type       Start            End               # Pages          Attributes\n");
		for (mm = MemMap; count < MemEntriesCount; mm=(EFI_MEMORY_DESCRIPTOR*)((UINT8*)mm+MemDescSize), count++) {
				addrEnd=mm->PhysicalStart+(UINT32)(mm->NumberOfPages)*0x1000-0x1;
				switch(mm->Type){
						case 0: Print(L"reserved   ");break;
						case 1: Print(L"LoaderData ");break;
						case 2: Print(L"LoaderCode ");break;
						case 3: Print(L"BS_data    ");break;
						case 4: Print(L"BS_code    ");break;
						case 5: Print(L"RT_data    ");break;
						case 6: Print(L"RT_code    ");break;
						case 7: Print(L"available  ");break;
						case 8: Print(L"unuseByOS  ");break;     
						case 9: Print(L"ACPI_recl  ");break;
						case 10: Print(L"ACPI_NVS   ");break;
						case 11: Print(L"MemMapIO   ");break;
						case 12: Print(L"PortMapIO  ");break;
						case 13: Print(L"PalCode    ");break;
						default: Print(L"Null");break;					
		        }
		        if (mm->Type == EfiLoaderCode || \
		            mm->Type == EfiLoaderData || \
		            mm->Type == EfiBootServicesCode || \
		            mm->Type == EfiBootServicesData || \
		            mm->Type == EfiRuntimeServicesCode || \
			    	mm->Type == EfiRuntimeServicesData || \
					mm->Type == EfiConventionalMemory || \
					mm->Type == EfiACPIReclaimMemory || \
					mm->Type == EfiPalCode || \
					mm->Type == EfiACPIMemoryNVS)
					{
						TotalPage += mm->NumberOfPages;
					}
				PageCnt[mm->Type] += mm->NumberOfPages;
				Print(L"%016llx-%016llx %016llx %016llx\n",mm->PhysicalStart,addrEnd,mm->NumberOfPages,mm->Attribute);
		}
		Print(L"\n");
		Print(L"  reserved  :%-7ld Pages ( %lld KB)\n",PageCnt[0],PageCnt[0]*4);//PageCnt[0]*0x1000
		Print(L"  LoaderCode:%-7ld Pages ( %lld KB)\n",PageCnt[1],PageCnt[1]*4);//PageCnt[1]*0x1000
		Print(L"  LoaderData:%-7ld Pages ( %lld KB)\n",PageCnt[2],PageCnt[2]*4);//PageCnt[2]*0x1000
		Print(L"  BS_code   :%-7ld Pages ( %lld KB)\n",PageCnt[3],PageCnt[3]*4);//PageCnt[3]*0x1000
		Print(L"  BS_data   :%-7ld Pages ( %lld KB)\n",PageCnt[4],PageCnt[4]*4);//PageCnt[4]*0x1000
		Print(L"  RT_code   :%-7ld Pages ( %lld KB)\n",PageCnt[5],PageCnt[5]*4);//PageCnt[5]*0x1000
		Print(L"  RT_data   :%-7ld Pages ( %lld KB)\n",PageCnt[6],PageCnt[6]*4);//PageCnt[6]*0x1000
		Print(L"  available :%-7ld Pages ( %lld KB)\n",PageCnt[7],PageCnt[7]*4);//PageCnt[7]*0x1000
		Print(L"  unuseByOS :%-7ld Pages ( %lld KB)\n",PageCnt[8],PageCnt[8]*4);//PageCnt[8]*0x1000
		Print(L"  ACPI_recl :%-7ld Pages ( %lld KB)\n",PageCnt[9],PageCnt[9]*4);//PageCnt[9]*0x1000
		Print(L"  ACPI_NVS  :%-7ld Pages ( %lld KB)\n",PageCnt[10],PageCnt[10]*4);//PageCnt[10]*0x1000
		Print(L"  MemMapIO  :%-7ld Pages ( %lld KB)\n",PageCnt[11],PageCnt[11]*4);//PageCnt[11]*0x1000
		Print(L"  PortMapIO :%-7ld Pages ( %lld KB)\n",PageCnt[12],PageCnt[12]*4);//PageCnt[12]*0x1000
		Print(L"  PalCode   :%-7ld Pages ( %lld KB)\n",PageCnt[13],PageCnt[13]*4);//PageCnt[13]*0x1000
		Print(L"\n");
		
		Print(L"Total Memory: %d MB ( %lld ) Bytes\n",TotalPage/256,TotalPage*0x1000);
		
		BS->FreePool(MemMap);
	}
	return EFI_SUCCESS; 
}

EFI_STATUS GhostEntryPoint(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	UINTN MemoryMapSize=0;
	EFI_MEMORY_DESCRIPTOR **MemoryMap;
	UINTN MapKey;
	UINTN DescriptorSize;
	UINT32 DescriptorVersion;
	EFI_STATUS status;
	UINT32 Multiple=1;
	*MemoryMap=NULL;
	
	InitializeShellApplication (ImageHandle, SystemTable);
	status=BS->GetMemoryMap(&MemoryMapSize,*MemoryMap,&MapKey,&DescriptorSize,&DescriptorVersion);
	Print(L"status is %08lx\n",status);
	Print(L"MemoryMapSize is %08lx\n",MemoryMapSize);
	Print(L"size of EFI_MEMORY_DESCRIPTOR is %08lx\n",sizeof(EFI_MEMORY_DESCRIPTOR));
	Print(L"MapKey is %08lx\n",MapKey);
	Print(L"DescriptorSize is %08lx\n",DescriptorSize);
	Print(L"DescriptorVersion is %08lx\n",DescriptorVersion);
	
	do{
			MemoryMapSize +=0x30;
			Multiple++;
			if(Multiple>5)break;
			status = BS->AllocatePool(EfiBootServicesData, MemoryMapSize, MemoryMap);
			if(EFI_ERROR(status))
			{
				Print(L"AllocatePool Error is %0llx\n",status);
				break;
			}
				status=BS->GetMemoryMap(&MemoryMapSize,*MemoryMap,&MapKey,&DescriptorSize,&DescriptorVersion);
				Print(L"Multiple is %08lx\n",Multiple);
				Print(L"status is %08lx\n",status);
				Print(L"MemoryMapSize is %08lx\n",MemoryMapSize);
				Print(L"size of EFI_MEMORY_DESCRIPTOR is %08lx\n",sizeof(EFI_MEMORY_DESCRIPTOR));
				Print(L"MapKey is %08lx\n",MapKey);
				Print(L"DescriptorSize is %08lx\n",DescriptorSize);
				Print(L"DescriptorVersion is %08lx\n",DescriptorVersion);
		}while(status == EFI_BUFFER_TOO_SMALL);
	
	
	
	return EFI_SUCCESS;
}
/*
UINT64 Shr64(
	IN UINT64 Value,
	IN UINT8 Shift
	)
{
	_asm {
		mov	edx, dword ptr Value[4]
		mov	eax, dword ptr Value
		mov	cl, Shift

		cmp	cl, 64
		jb	less_64
		xor	eax, eax
		xor	edx, edx
		jmp	exit
less_64:
		cmp	cl, 32			//Shift is 32 modulo
		jb less_32

		mov		eax, edx
		xor		edx, edx
less_32:
		shrd	eax, edx, cl
		shr		edx, cl
exit:
	}
}
UINT64 bit64_shr(IN UINT64 Value,IN UINT8 Shift)
{
	UINT64 cBack=0;
	_asm {
		mov edx, dword ptr Value[4]
		mov eax, dword ptr Value
		mov cl, Shift	
		cmp cl, 64
		jb less_64
		xor	eax, eax
		xor	edx, edx
		jmp	exit
less_64:
		cmp cl, 32
		jb less_32

		mov edx, eax
		xor edx, edx
less_32:
		
exit:
	}
	return 0;
}*/
void PrintMemType(unsigned char MemType)
{
	switch(MemType)
	{
		case 0x00: Print(L" UC "); break;
		case 0x01: Print(L" WC "); break;
		case 0x04: Print(L" WT "); break;
		case 0x05: Print(L" WP "); break;
		case 0x06: Print(L" WB "); break;
		default:   Print(L" RV ");break;
	}
}
void FixPrint(UINT32 *fixMTRR,UINT32 len)
{
	UINT32 value;
	char *memType;
	UINT8 bit3=0;
	UINT32 i;
	for(i=0;i<len;i++)
	{
		value=EfiReadMsr(fixMTRR[i]);
		bit3=(UINT8)(value & 0x07);
		switch(i)
		{
			case 0:Print(L"FIX64K_00000 00000h to 7FFFFh    "); break;
			case 1:Print(L"FIX16K_80000 80000h to 9FFFFh    "); break;
			case 2:Print(L"FIX16K_A0000 A0000h to BFFFFh    "); break;
			case 3:Print(L"FIX4K_C0000  C0000h to C7FFFh    "); break;
			case 4:Print(L"FIX4K_C8000  C8000h to CFFFFh    "); break;
			case 5:Print(L"FIX4K_D0000  D0000h to D7FFFh    "); break;
			case 6:Print(L"FIX4K_D8000  D8000h to DFFFFh    "); break;
			case 7:Print(L"FIX4K_E0000  E0000h to E7FFFh    "); break;
			case 8:Print(L"FIX4K_E8000  E8000h to EFFFFh    "); break;
			case 9:Print(L"FIX4K_F0000  F0000h to F7FFFh    "); break;
		   case 10:Print(L"FIX4K_F8000  F8000h to FFFFFh    "); break;
			default:break;
		}
		PrintMemType(bit3);
		Print(L"\n");	
	}	
}
EFI_STATUS ClareAllocatePages(EFI_PHYSICAL_ADDRESS *Memory,UINT32 *Pages)
{
	EFI_ALLOCATE_TYPE Type=AllocateAnyPages;
	EFI_MEMORY_TYPE MemoryType=EfiLoaderData;
	//UINTN Pages=512;
	//EFI_PHYSICAL_ADDRESS Memory=0;
	EFI_STATUS Status;
	Status = BS->AllocatePages(Type,MemoryType,*Pages,Memory);
	switch(Status)
	{
		case EFI_SUCCESS:break;
		case EFI_OUT_OF_RESOURCES:Print(L"The pages could not be allocated.");break;
		case EFI_INVALID_PARAMETER:Print(L"Type is not AllocateAnyPages or AllocateMaxAddress or AllocateAddress.");break;
		case EFI_NOT_FOUND:Print(L"The requested pages could not be found.");break;
		default:break;
	}
	return Status;
//	Print(L"\nMemory Addr is %llx\n",*Memory);
	
}
void ClareFreePages(EFI_PHYSICAL_ADDRESS *Memory,UINT32 *Pages)
{
	BS->FreePages(*Memory,*Pages);
}
EFI_STATUS GangEntryPoint(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	UINT32 count=0;
	UINT32 pAddrStart=0x200;
	UINT32 pBaseIndex=pAddrStart;
	UINT32 pMaskIndex=pBaseIndex+1;
	UINT64 baseValue=0,maskValue=0;
	UINT64 storeBaseValue=0,storeMaskValue=0;
	UINT64 startAddr=0,endAddr=0;
	UINT64 yuMask=0xFFFFFFFFFFFFFFFF;
	UINT32 fixMTRR[11]={0x250,0x258,0x259,0x268,0x269,0x26A,0x26B,0x26C,0x26D,0x26E,0x26F};
	unsigned char PhyAddrSize=0;
	EFI_STATUS status;
	UINT32 RegisterInEax=0x80000008,MemType;
	EFI_CPUID_REGISTER Reg;
	UINT64 i=0,iv=1;
	
	UINT64 timeCount1=0,timeCount2=0,timeBridge;
	UINT64 cpuFreq;
	
	EFI_PHYSICAL_ADDRESS Memory,mm,mm0;
	UINT32 Pages,needToFreePages;
	UINT64 allocBase=0,allocMask=0;
	UINT64 maTime=0;
	UINT64 maSpeed=0;
	UINT8 mtArray[3]={0x00,0x04,0x06};
	UINT8 mtArrayIndex=2;
	unsigned char cdata=0;
	UINT8 cExist=0;
	InitializeShellApplication (ImageHandle, SystemTable);

	EfiCpuid(RegisterInEax, &Reg);
	PhyAddrSize=Reg.RegEax & 0x000000ff;
	Print(L"RegEax = %08lx   \n",Reg.RegEax);
	Print(L"PhyAddrSize = %d   \n",PhyAddrSize);
	Print(L"***********************  gang mode  ****************************\n");
	yuMask = (yuMask>>(64-PhyAddrSize+12))<<12;
	while(pBaseIndex < pAddrStart+2*10)
	{
		count++;
		baseValue=EfiReadMsr(pBaseIndex);
		maskValue=EfiReadMsr(pMaskIndex);

		if(maskValue & (0x001<<11))
		{		
			Print(L"%d",count);
			Print(L" MSR[%03lx,%03lx] ",pBaseIndex,pMaskIndex);
			Print(L"%016llx ",baseValue);
			Print(L"%016llx ",maskValue);
			MemType = baseValue & 0x07;
			startAddr = baseValue & yuMask;
			endAddr=maskValue & yuMask;
			endAddr=~(endAddr);
			endAddr=endAddr & yuMask;
			endAddr=endAddr+(1<<12);
			endAddr = startAddr+endAddr-1;
			Print(L"%016llx ",startAddr);
			Print(L"%016llx ",endAddr);
			switch(MemType)
			{
				case 0x00: Print(L" UC "); break;
				case 0x01: Print(L" WC "); break;
				case 0x04: Print(L" WT "); break;
				case 0x05: Print(L" WP "); break;
				case 0x06: Print(L" WB "); break;
				default:break;
			}
		}
		else
		{
			cExist=1;
			Print(L"\n");
			break;
		}
		Print(L"\n");
		pBaseIndex+=2;
		pMaskIndex+=2;
	}
	if(cExist==0)
	{
		pBaseIndex-=2;
		pMaskIndex-=2;
		storeBaseValue=EfiReadMsr(pBaseIndex);
		storeMaskValue=EfiReadMsr(pMaskIndex);
	}
	
	FixPrint(fixMTRR,11);
	Print(L"*******************  Part Two  *************************\n");
	
	timeCount1=EfiReadTsc();
	BS->Stall(6000);
	timeCount2=EfiReadTsc();
	cpuFreq=(timeCount2-timeCount1)/(6*1000);
	Print(L"\ncpuFreq is %ld MHz\n",cpuFreq);

	Print(L"*******************  Part Three  *************************\n");
	mtArrayIndex=0;
	while(mtArrayIndex<3)
	{

		Pages=256*8;
		needToFreePages=Pages;
		status=ClareAllocatePages(&Memory,&Pages);
		if(EFI_ERROR(status))
		{
			Print(L"\nSome Error Happened\n",Memory);
			return EFI_SUCCESS;
		}
		Print(L"\nMemory Addr is %llx\n",Memory);
		allocBase=Memory | mtArray[mtArrayIndex];
		EfiWriteMsr(pBaseIndex,allocBase);
		Pages=(Pages<<12)-1;
		Pages=(~Pages) & yuMask;
		allocMask=Pages | 0x0800;
		EfiWriteMsr(pMaskIndex,allocMask);
		mm = Memory;
		mm = mm & 0xFFFFFFFFFFF00000;
		mm = mm + 0x100000;
		mm0=mm;
		Print(L"Address Found : %016llx    ",mm);
		switch(mtArrayIndex)
		{
			case 0:Print(L"UC\n",mm);break;
			case 1:Print(L"WT\n",mm);break;
			case 2:Print(L"WB\n",mm);break;
			default:Print(L"ERROR\n",mm);break;
		}
		Print(L"MSR[%x]=%016llx   ",pBaseIndex,allocBase);
		Print(L"MSR[%x]=%016llx   \n",pMaskIndex,allocMask);
		//Write Miss
		Print(L"W: Miss - ");
		EfiDisableCache();
		EfiEnableCache();
		timeCount1=EfiReadTsc();
		mm=mm0;
		while(mm<mm0+512*4096)
		{
			*((UINT16 *)mm)=1;
			mm++;mm++;
		}
		timeCount2=EfiReadTsc();
		timeBridge=timeCount2-timeCount1;
		maSpeed=2*cpuFreq*1000*1000/timeBridge/2;
		Print(L"%ld MB/s",maSpeed);
		Print(L", Hit - ");
		//Write Hit
		timeCount1=EfiReadTsc();
		mm=mm0;
		while(mm<mm0+512*4096)
		{
			*((UINT16 *)mm)=1;
			mm++;mm++;
		}
		timeCount2=EfiReadTsc();
		timeBridge=timeCount2-timeCount1;
		maSpeed=2*cpuFreq*1000*1000/timeBridge/2;
		Print(L"%ld MB/s\n",maSpeed);
		
		//Read Miss
		EfiDisableCache();
		EfiEnableCache();
		Print(L"R: Miss - ");
		timeCount1=EfiReadTsc();
		mm=mm0;
		while(mm<mm0+512*4096)
		{
			cdata=*((UINT16 *)mm);
			mm++;mm++;
		}
		timeCount2=EfiReadTsc();
		timeBridge=timeCount2-timeCount1;
		maSpeed=2*cpuFreq*1000*1000/timeBridge/2;
		Print(L"%ld MB/s",maSpeed);
		Print(L", Hit - ");
		//Read Hit
		timeCount1=EfiReadTsc();
		mm=mm0;
		while(mm<mm0+512*4096)
		{
			cdata=*((UINT16 *)mm);
			mm++;mm++;
		}
		timeCount2=EfiReadTsc();
		timeBridge=timeCount2-timeCount1;
		maSpeed=2*cpuFreq*1000*1000/timeBridge/2;
		Print(L"%ld MB/s\n",maSpeed);
		
		ClareFreePages(&Memory,&needToFreePages);
		mtArrayIndex++;
	}
	if(cExist==0)
	{
		EfiWriteMsr(pBaseIndex,storeBaseValue);
		EfiWriteMsr(pMaskIndex,storeMaskValue);
	}
	else
	{
		EfiWriteMsr(storeBaseValue,0x0);
		EfiWriteMsr(storeMaskValue,0x0);
	}

	return EFI_SUCCESS;
}
EFI_STATUS Bit64EntryPoint(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	UINT64 cData=0xffff123456780000;
	InitializeShellApplication (ImageHandle, SystemTable);
	
	Print(L"****************  Begin  ************************\n");
	Print(L"source cData is %016llx\n",cData);

	cData=EfiShrBit64(12,cData);
	Print(L"shr cData is %016llx\n",cData);

	cData=0xffff123456780000;
	cData=EfiAddBit64(0x5788,cData);
	Print(L"add cData is %016llx\n",cData);
	Print(L"****************   End   ************************\n");

	cData=0xffff123456780000;
	cData=EfiRetBit64(cData);
	Print(L"add cData is %016llx\n",cData);
	Print(L"****************   End   ************************\n");

	return EFI_SUCCESS;
}