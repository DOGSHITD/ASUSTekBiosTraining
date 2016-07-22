//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************

//*************************************************************************
// $Header: /Alaska/SOURCE/Core/Modules/IA32Core/IA32CLib.c
//
//<AMI_FHDR_END>
//*************************************************************************
#include <efi.h>
#include <efilib.h>
#include <CpuLib.h>

UINT8 IoRead8(UINT16 Port)
{
	_asm {
		mov dx, Port
		in al, dx
	}
}

UINT16 IoRead16(UINT16 Port)
{
	_asm {
		mov dx, Port
		in ax, dx
	}
}

UINT32 IoRead32(UINT16 Port)
{
	_asm {
		mov dx, Port
		in eax, dx
	}
}

void IoWrite8(UINT16 Port,UINT8 Value)
{
	_asm {
		mov dx, Port
		mov	al, Value
		out dx, al
	}
}

void IoWrite16(UINT16 Port,UINT16 Value)
{
	_asm {
		mov dx, Port
		mov	ax, Value
		out dx, ax
	}
}

void IoWrite32(UINT16 Port,UINT32 Value)
{
	_asm {
		mov dx, Port
		mov	eax, Value
		out dx, eax
	}
}
 
//------------------------------------------------------------------------------
//  EfiHalt
//------------------------------------------------------------------------------
void EfiHalt()
{
	_asm {
		hlt
	}
}

//------------------------------------------------------------------------------
//  EfiWbinvd
//------------------------------------------------------------------------------
void EfiWbinvd()
{
	_asm {
		wbinvd
	}
}

//------------------------------------------------------------------------------
// EfiInvd
//------------------------------------------------------------------------------
void EfiInvd()
{
	_asm {
		invd
	}
}

//------------------------------------------------------------------------------
//  CPULib_CpuID
//------------------------------------------------------------------------------
VOID CPULib_CpuID(
    IN UINT32 CpuIDIndex, 
    IN OUT UINT32 *pRegEAX, 
    IN OUT UINT32 *pRegEBX, 
    IN OUT UINT32 *pRegECX, 
    IN OUT UINT32 *pRegEDX)
{
    _asm{
        push ebx
        push ecx
        push edx
        push esi
        mov esi, pRegECX
        mov	ecx, [esi]
        mov eax, CpuIDIndex
        cpuid
        mov esi, pRegEAX
        or  esi, esi
        jz  skip1
        mov	[esi], eax
skip1:
        mov	esi, pRegEBX
        or  esi, esi
        jz  skip2
        mov	[esi], ebx
skip2:
        mov	esi, pRegECX
        or  esi, esi
        jz  skip3
        mov	[esi], ecx
skip3:
        mov	esi, pRegEDX
        or  esi, esi
        jz  skip4
        mov	[esi], edx
skip4:
        pop	esi
        pop	edx
        pop	ecx
        pop ebx
    }
}

//------------------------------------------------------------------------------
// EfiCpuid
//------------------------------------------------------------------------------
VOID EfiCpuid(UINT32 RegisterInEax, EFI_CPUID_REGISTER *Reg)
{
	UINT32	Eax, Ebx, Ecx, Edx;
	//Print(L"RegisterInEax = %08lx \n",RegisterInEax);
	CPULib_CpuID(RegisterInEax, &Eax, &Ebx, &Ecx, &Edx);
	
	/*Print(L"Eax = %08lx \n",Eax);
	Print(L"Ebx = %08lx \n",Ebx);
	Print(L"Ecx = %08lx \n",Ecx);
	Print(L"Edx = %08lx \n",Edx);*/

	Reg->RegEax = Eax;
	Reg->RegEbx = Ebx;
	Reg->RegEcx = Ecx;
	Reg->RegEdx = Ebx;
		
}

//------------------------------------------------------------------------------
// EfiReadMsr
//------------------------------------------------------------------------------
UINT64 EfiReadMsr(IN UINT32 Msr){
    _asm{
        mov	ecx, Msr                 ;MSR register
        rdmsr
    }
}
//------------------------------------------------------------------------------
//  EfiWriteMsr
//------------------------------------------------------------------------------
VOID EfiWriteMsr(IN UINT32 Msr, IN UINT64 Value){
    _asm{
        mov ecx, Msr					;MSR register
        mov edx, dword ptr Value[4]     ;Upper 32 bit MSR Value
        mov eax, dword ptr Value[0]     ;Lower 32 bit MSR Value
        wrmsr
    }
}

//------------------------------------------------------------------------------
// EfiReadTsc
//------------------------------------------------------------------------------
UINT64 EfiReadTsc(VOID)
{
    _asm{
        rdtsc
    }
}
//------------------------------------------------------------------------------
// EfiDisableCache
//------------------------------------------------------------------------------
VOID EfiDisableCache(VOID)
{
    _asm{
        wbinvd
        mov	eax, cr0
        or	eax, 060000000h		;SET CD, NW
        mov	cr0, eax
        wbinvd					;Invalidate cache
    }
}

//------------------------------------------------------------------------------
// EfiEnableCache
//------------------------------------------------------------------------------
VOID EfiEnableCache(VOID)
{
    _asm{
    // Enable cache
        mov	eax, cr0
        and	eax, 09fffffffh		;SET CD, NW
        mov	cr0, eax
        wbinvd
    }
}

//------------------------------------------------------------------------------
// EfiDisableInterrupts
//------------------------------------------------------------------------------
VOID EfiDisableInterrupts(VOID){
    _asm{
        cli
    }
}

//------------------------------------------------------------------------------
// EfiEnableInterrupts
//------------------------------------------------------------------------------
VOID EfiEnableInterrupts(VOID){
    _asm{
        sti
    }
}

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
