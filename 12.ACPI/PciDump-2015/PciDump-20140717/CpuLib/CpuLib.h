#ifndef __CPULIB_H__
#define __CPULIB_H__

typedef struct {
  UINT32  RegEax;
  UINT32  RegEbx;
  UINT32  RegEcx;
  UINT32  RegEdx;
} EFI_CPUID_REGISTER;

UINT64 EfiReadMsr(UINT32 Msr);
VOID EfiWriteMsr(UINT32 Msr, UINT64 Value);
UINT64 EfiReadTsc();
VOID EfiCpuid(UINT32 RegisterInEax, EFI_CPUID_REGISTER *Reg);
VOID EfiEnableCache();
VOID EfiDisableCache();

//================================================================
// IO Access
//================================================================
UINT8	IoRead8(UINT16 Port);
UINT16	IoRead16(UINT16 Port);
UINT32	IoRead32(UINT16 Port);

VOID	IoWrite8(UINT16 Port, UINT8 Value);
VOID	IoWrite16(UINT16 Port, UINT16 Value);
VOID	IoWrite32(UINT16 Port, UINT32 Value);

//================================================================
// Memory Access
//================================================================
#define	 MemRead8(Address)			*((UINT8 *)((UINT32)Address))
#define	 MemRead16(Address)			*((UINT16 *)((UINT32)Address))
#define	 MemRead32(Address)			*((UINT32 *)((UINT32)Address))

#define	 MemWrite8(Address, Data)		*((UINT8 *)((UINT32)Address))=Data
#define	 MemWrite16(Address, Data)		*((UINT16 *)((UINT32)Address))=Data
#define	 MemWrite32(Address, Data)		*((UINT32 *)((UINT32)Address))=Data

UINT64 Div64 (UINT64 Dividend, UINTN Divisor, UINTN	*Remainder);

#endif //#ifndef __CPULIB_H__