//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2007, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**             6145-F Northbelt Pkwy, Norcross, GA 30071            **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
//**********************************************************************
//
// $Header: /Alaska/Projects/Intel/Eaglelake/Eaglelake_ICH10_1ADXS/CRB/CRBSMI.c 1     1/17/08 5:18a Tonywu $
//
// $Revision: 1 $
//
// $Date: 1/17/08 5:18a $
//
//**********************************************************************
// Revision History
// ----------------
// $Log: /Alaska/Projects/Intel/Eaglelake/Eaglelake_ICH10_1ADXS/CRB/CRBSMI.c $
// 
// 1     1/17/08 5:18a Tonywu
// 
//**********************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:        CRBSMI.c
//
// Description: This file contains code for all CRB SMI events
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

//----------------------------------------------------------------------------
// Include(s)
//----------------------------------------------------------------------------

#include <Token.h>
#include <AmiLib.h>
#include <AmiDxeLib.h>
#include <AmiCspLib.h>
#include "Protocol\SmmSwDispatch.h"
#include "Protocol\SmmSxDispatch.h"
#include "Protocol\SmmPowerButtonDispatch.h"

// Produced Protocols
#include <setup.h>
//lmrtestFixMakeErrorForAMI_SuperIO #include "SIOBasicIODXE.h"
// Consumed Protocols

//----------------------------------------------------------------------------
// Constant, Macro and Type Definition(s)
//----------------------------------------------------------------------------
// Constant Definition(s)

#ifndef ACPI_SUPPORT
#define ACPI_SUPPORT                    0
#endif

// Macro Definition(s)

// Type Definition(s)

// Function Prototype(s)

//----------------------------------------------------------------------------
// Variable and External Declaration(s)
//----------------------------------------------------------------------------
// Variable Declaration(s)


// GUID Definition(s)
static SETUP_DATA	*SetupDataPtr = NULL;
static SETUP_DATA	SetupData;
// Protocol Definition(s)

// External Declaration(s)
extern VOID S5_Enable_PME();
extern VOID S5_Disable_PME();
// Function Definition(s)

//----------------------------------------------------------------------------
//lmrtestFixMakeErrorForAMI_SuperIO static EFI_ASUS_SIO_BIO_DXE			*SIO_BIO = NULL;
//ASUS-Normduke-test-	static EFI_ASUS_SIO_BIO_DXE			data_SIO_BIO;
//<ASUS-LMR110322+>
#ifdef NCT6791D_SUPPORT
 #if NCT6791D_SUPPORT
VOID	SIOBasicIORead(UINT8 nLDN, UINT8 nOffset,UINT8* nValue){
	IoWrite8(NCT6791D_CONFIG_INDEX, 0x87);
	IoWrite8(NCT6791D_CONFIG_INDEX, 0x87);

	IoWrite8(NCT6791D_CONFIG_INDEX, NCT6791D_LDN_SEL_REGISTER);
	IoWrite8(NCT6791D_CONFIG_DATA, nLDN);		//Select device
	IoWrite8(NCT6791D_CONFIG_INDEX, nOffset);
	*nValue=IoRead8(NCT6791D_CONFIG_DATA);		//read register value

	IoWrite8(NCT6791D_CONFIG_INDEX, 0xaa);
}

VOID	SIOBasicIOWrite(UINT8 nLDN, UINT8 nOffset,UINT8 nValue){
	IoWrite8(NCT6791D_CONFIG_INDEX, 0x87);
	IoWrite8(NCT6791D_CONFIG_INDEX, 0x87);

	IoWrite8(NCT6791D_CONFIG_INDEX, NCT6791D_LDN_SEL_REGISTER);
	IoWrite8(NCT6791D_CONFIG_DATA, nLDN);		// Select device
	IoWrite8(NCT6791D_CONFIG_INDEX, nOffset);
	IoWrite8(NCT6791D_CONFIG_DATA, nValue);		//write register value

	IoWrite8(NCT6791D_CONFIG_INDEX, 0xaa);
}

VOID	IOSetBit(UINT8 nLDN, UINT8 nOffset,UINT8 nData){
	IoWrite8(NCT6791D_CONFIG_INDEX, 0x87);
	IoWrite8(NCT6791D_CONFIG_INDEX, 0x87);

	IoWrite8(NCT6791D_CONFIG_INDEX, NCT6791D_LDN_SEL_REGISTER);
	IoWrite8(NCT6791D_CONFIG_DATA, nLDN);		// Select device
	IoWrite8(NCT6791D_CONFIG_INDEX, nOffset);
	IoWrite8(NCT6791D_CONFIG_DATA, IoRead8(NCT6791D_CONFIG_DATA)|nData);	//write register value

	IoWrite8(NCT6791D_CONFIG_INDEX, 0xaa);
}

VOID	IOResetBit(UINT8 nLDN, UINT8 nOffset,UINT8 nData){
	IoWrite8(NCT6791D_CONFIG_INDEX, 0x87);
	IoWrite8(NCT6791D_CONFIG_INDEX, 0x87);

	IoWrite8(NCT6791D_CONFIG_INDEX, NCT6791D_LDN_SEL_REGISTER);
	IoWrite8(NCT6791D_CONFIG_DATA, nLDN);		// Select device
	IoWrite8(NCT6791D_CONFIG_INDEX, nOffset);
	IoWrite8(NCT6791D_CONFIG_DATA, IoRead8(NCT6791D_CONFIG_DATA)&~nData);	//write register value

	IoWrite8(NCT6791D_CONFIG_INDEX, 0xaa);
}
 #endif
#endif //NCT6791D_SUPPORT
//<ASUS-LMR110322->

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   PreparePotocolForUse
//
// Description: This is a template for prepate potocol into global variable.
//
// Parameters:  DispatchHandle  - EFI Handle
//              DispatchContext - Pointer to the EFI_SMM_SX_DISPATCH_CONTEXT
//
// Returns:     None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID	PreparePotocolForUse(
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable )
{
//lmrtestFixMakeErrorForAMI_SuperIO	EFI_GUID	gSIOBIO = EFI_DXE_ASUS_SUPER_IO_BIO_GUID;
	EFI_GUID	gSetupGuid = SETUP_GUID;
	EFI_STATUS	Status;
	UINTN		VariableSize = NULL;

//lmrtestFixMakeErrorForAMI_SuperIO	Status  = pBS->LocateProtocol( &gSIOBIO, NULL, &SIO_BIO );
//lmrtestFixMakeErrorForAMI_SuperIO	if(EFI_ERROR(Status))	SIO_BIO = NULL;

//ASUS-Normduke-test-		pBS->CopyMem(&data_SIO_BIO,SIO_BIO,sizeof(EFI_ASUS_SIO_BIO_DXE));
//ASUS-Normduke-test-	        SIO_BIO=&data_SIO_BIO;

	VariableSize = sizeof(SETUP_DATA);

	Status = pRS->GetVariable(
		L"Setup",
		&gSetupGuid,
		NULL,
		&VariableSize,
		&SetupData);
	ASSERT_EFI_ERROR(Status);

	if(!EFI_ERROR(Status))
		SetupDataPtr=&SetupData;
}

VOID	Io_Delay(UINT16	count)
{
	for((UINT16)count;(UINT16)count>0;(UINT16)count--)
		IoWrite8(0xE1, 0x00);
}

VOID Clear8042OutputBuffer()
{
	UINT16 counter = 1000;

	while((UINT16)counter-- > 0) {
		if(!(IoRead8(0x64) & 0x01))
			break;

		Io_Delay(1);
//<ASUS-LMR100519A->		IoRead8(0x60);
		if ((IoRead8(0x60)==0xff) && (IoRead8(0x64)==0xff)) break;	//<ASUS-LMR100519A+> fix system take long time to shutdown if press power button during POST. (keyboard controller is not enabled yet.)
		Io_Delay(0x2800);
	}
}



VOID	CheckS1PS2KeyboardWakeUp()
{
//<ASUS-LMR110609+>
#ifdef NCT6791D_SUPPORT
 #if NCT6791D_SUPPORT
	UINT8	Data;
	BOOLEAN	bKeyboardWakeUp=FALSE,bMouseWakeUp=FALSE;
	SIOBasicIORead(0x0A, 0xE3, &Data);	//ClearStatus

////<ASUS-LMR110610->	Data = IoRead8(PM_BASE_ADDRESS+ICH_IOREG_GPE0_EN+2);
	SIOBasicIORead(0x0A, 0xF6, &Data);		//<ASUS-LMR110610+> check SIO register instead of GPE due to PCITree.ASL PWRB patch code caused GPE always=1
	bKeyboardWakeUp=((Data&BIT4)!=0);
////<ASUS-LMR110610->	Data = IoRead8(PM_BASE_ADDRESS+ICH_IOREG_GPE0_EN+3);
	bMouseWakeUp=((Data&BIT5)!=0);

	if ((!(bKeyboardWakeUp&&bMouseWakeUp)) && 
		(bKeyboardWakeUp||bMouseWakeUp))
	{
		if (bKeyboardWakeUp)
			IoWrite8(0x64,0xa7);		//Disable Auxiliary device interface
		else
			IoWrite8(0x64,0xad);		//Disable keyboard device interface
		Io_Delay(2);
		Data=IoRead8(0x60);
	}

	if(bKeyboardWakeUp||bMouseWakeUp)
	{
		IOSetBit(0x0A, 0xF2, BIT0);		//Enable SIO PME# for sure
		SIOBasicIOWrite(0x0A, 0xF3, 0xFF);
		SIOBasicIOWrite(0x0A, 0xF4, 0xFF);
		if (bKeyboardWakeUp)
			IOSetBit(0x0A, 0xF6, BIT4);
		else
			IOResetBit(0x0A, 0xF6, BIT4);
		if (bMouseWakeUp)
			IOSetBit(0x0A, 0xF6, BIT5);
		else
			IOResetBit(0x0A, 0xF6, BIT5);
		IoWrite32(PM_BASE_ADDRESS+ICH_IOREG_GPE0_EN,IoRead32(PM_BASE_ADDRESS+ICH_IOREG_GPE0_EN)|BIT29);
//<ASUS-LMR110610->		IoWrite32(PM_BASE_ADDRESS+ICH_IOREG_GPE0_EN,IoRead32(PM_BASE_ADDRESS+ICH_IOREG_GPE0_EN)&(~BIT22));	//Disable dummy GPE#
	}
	else
	{
		IOResetBit(0x0A, 0xF6, BIT4);
		IOResetBit(0x0A, 0xF6, BIT5);
		IoWrite32(PM_BASE_ADDRESS+ICH_IOREG_GPE0_EN,IoRead32(PM_BASE_ADDRESS+ICH_IOREG_GPE0_EN)&(~BIT29));	//<ASUS-LMR110610+>
	}
 #endif //NCT6791D_SUPPORT
#endif //NCT6791D_SUPPORT
//<ASUS-LMR110609->
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   ProjectS1SmiHandler
//
// Description: This is a template Project Sx SMI Handler for Porting.
//
// Parameters:  DispatchHandle  - EFI Handle
//              DispatchContext - Pointer to the EFI_SMM_SX_DISPATCH_CONTEXT
//
// Returns:     None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID ProjectS1SmiHandler (
    IN EFI_HANDLE                   DispatchHandle,
    IN EFI_SMM_SX_DISPATCH_CONTEXT  *DispatchContext )
{
    // Porting if needed
	IoWrite8(GPIO_BASE_ADDRESS+GP_IOREG_GPO_BLINK+3,(IoRead8(GPIO_BASE_ADDRESS+GP_IOREG_GPO_BLINK+3)|0x01));	//Blinking power LED
	Clear8042OutputBuffer();
	CheckS1PS2KeyboardWakeUp();
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   ProjectS1ResumeSmiHandler
//
// Description: This is a template Project Sx SMI Handler for Porting.
//
// Parameters:  DispatchHandle  - EFI Handle
//              DispatchContext - Pointer to the EFI_SMM_SX_DISPATCH_CONTEXT
//
// Returns:     None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID ProjectS1ResumeSmiHandler (
    IN EFI_HANDLE                   DispatchHandle,
    IN EFI_SMM_SX_DISPATCH_CONTEXT  *DispatchContext )
{
    // Porting if needed
	IoWrite8(GPIO_BASE_ADDRESS+GP_IOREG_GPO_BLINK+3,(IoRead8(GPIO_BASE_ADDRESS+GP_IOREG_GPO_BLINK+3)&(~0x01)));		//<ASUS-LMR110609+>
}
//<ASUS-Terry2_Wang140708+>
VOID PortRestoreACPowerLoss(){
//<ASUS-DTL20110516+>
//<ASUS-LMR100422+> Support ACPLR controlled by SuperI/O and WOL after G3 function.
#ifdef NCT6791D_SUPPORT
 #if NCT6791D_SUPPORT
//#include <Build\Board\IO\W83677HGPEI.H>
	UINT16	Value;

	IoWrite8(NCT6791D_CONFIG_INDEX, 0x87);
	IoWrite8(NCT6791D_CONFIG_INDEX, 0x87);
	IoWrite8(NCT6791D_CONFIG_INDEX, NCT6791D_LDN_SEL_REGISTER);
	IoWrite8(NCT6791D_CONFIG_DATA, NCT6791D_LDN_ACPI);
	if ((SetupDataPtr->LastState == 0)||(SetupDataPtr->LastState == 2)){	//Off or Previous state?
		if(SetupDataPtr->PowerupPCIE){	//Patch PCH GbE WOL after G3	
			IoWrite8(NCT6791D_CONFIG_INDEX, 0xE4);
			IoWrite8(NCT6791D_CONFIG_DATA, IoRead8(NCT6791D_CONFIG_DATA)&(~BIT6)|BIT5); //Set RxE4h[6:5]=01b
			IoWrite8(NCT6791D_CONFIG_INDEX, 0xE7);
			IoWrite8(NCT6791D_CONFIG_DATA, IoRead8(NCT6791D_CONFIG_DATA)&(~BIT4)); //Set RxE7h[4]=0
//			SET_PCI8_SB(ICH_REG_GEN_PMCON_3, 1); //set SB RxA4[0]=1, RxA4[13]=1
		        IoWrite32(0xcf8, BIT31 | (SB_BUS << 16) | (SB_DEV << 11) | (SB_FUN << 8) | (SB_REG_GEN_PMCON_3 & 0xfc));
	        	Value = IoRead16(0xcfc | (UINT8)(SB_REG_GEN_PMCON_3 & 3));
		        Value|=BIT0+BIT13;
	        	IoWrite16(0xcfc | (UINT8)(SB_REG_GEN_PMCON_3 & 3), Value);

		} else {	//WOL disabled, ACPLR Off

			IoWrite8(NCT6791D_CONFIG_INDEX, 0xE4);
			IoWrite8(NCT6791D_CONFIG_DATA, IoRead8(NCT6791D_CONFIG_DATA)&(~(BIT5+BIT6))); //Set RxE4h[6:5]=00b
			IoWrite8(NCT6791D_CONFIG_INDEX, 0xE7);
			IoWrite8(NCT6791D_CONFIG_DATA, IoRead8(NCT6791D_CONFIG_DATA)|BIT4); //Set RxE7h[4]=1
//			SET_PCI8_SB(ICH_REG_GEN_PMCON_3, 1); //set SB RxA4[0]=1, RxA4[13]=0
		        IoWrite32(0xcf8, BIT31 | (SB_BUS << 16) | (SB_DEV << 11) | (SB_FUN << 8) | (SB_REG_GEN_PMCON_3 & 0xfc));
	        	Value = IoRead16(0xcfc | (UINT8)(SB_REG_GEN_PMCON_3 & 3));
		        Value|=BIT0;
				Value&=(~BIT13);
	        	IoWrite16(0xcfc | (UINT8)(SB_REG_GEN_PMCON_3 & 3), Value);
		}
//<ASUS-Rick1_Chung111116>		}; //end if else
	} else if (SetupDataPtr->LastState == 1){	//ON
			IoWrite8(NCT6791D_CONFIG_INDEX, 0xE4);
			IoWrite8(NCT6791D_CONFIG_DATA, IoRead8(NCT6791D_CONFIG_DATA)&(~BIT6)|BIT5); //Set RxE4h[6:5]=01b
			IoWrite8(NCT6791D_CONFIG_INDEX, 0xE7);
			IoWrite8(NCT6791D_CONFIG_DATA, IoRead8(NCT6791D_CONFIG_DATA)|BIT4); //Set RxE7h[4]=1
//			SET_PCI8_SB(ICH_REG_GEN_PMCON_3, 1); //set SB RxA4[0]=0, RxA4[13]=0
		        IoWrite32(0xcf8, BIT31 | (SB_BUS << 16) | (SB_DEV << 11) | (SB_FUN << 8) | (SB_REG_GEN_PMCON_3 & 0xfc));
	        	Value = IoRead16(0xcfc | (UINT8)(SB_REG_GEN_PMCON_3 & 3));
		        Value=Value&(~(BIT0+BIT13));
	        	IoWrite16(0xcfc | (UINT8)(SB_REG_GEN_PMCON_3 & 3), Value);
	}; //end if else
	IoWrite8(NCT6791D_CONFIG_INDEX, SIO_CONFIG_MODE_EXIT_VALUE);
 #endif //NCT6791D_SUPPORT
#endif //NCT6791D_SUPPORT
//<ASUS-LMR100422->
//<ASUS-DTL20110516->

	return;
}
//<ASUS-Terry2_Wang140708->
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   ProjectS3SmiHandler
//
// Description: This is a template Project Sx SMI Handler for Porting.
//
// Parameters:  DispatchHandle  - EFI Handle
//              DispatchContext - Pointer to the EFI_SMM_SX_DISPATCH_CONTEXT
//
// Returns:     None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID ProjectS3SmiHandler (
    IN EFI_HANDLE                   DispatchHandle,
    IN EFI_SMM_SX_DISPATCH_CONTEXT  *DispatchContext )
{
	Clear8042OutputBuffer();
	//CheckPS2KeyboardWakeUp();
	IoWrite8(GPIO_BASE_ADDRESS+GP_IOREG_IO_SEL+3,(IoRead8(GPIO_BASE_ADDRESS+GP_IOREG_IO_SEL+3)&(~0x01)));	//Blinking power LED(GPIO24)
	IoWrite8(GPIO_BASE_ADDRESS+GP_IOREG_GP_LVL+3,(IoRead8(GPIO_BASE_ADDRESS+GP_IOREG_GP_LVL+3)|0x01));	//Blinking power LED(GPIO24)

	IoWrite8(GPIO_BASE_ADDRESS+GP_IOREG_GPO_BLINK+3,(IoRead8(GPIO_BASE_ADDRESS+GP_IOREG_GPO_BLINK+3)|0x01));	//Blinking power LED(GPIO24)
//lmrtestFixMakeErrorForAMI_SuperIO	if(SIO_BIO) 
//lmrtestFixMakeErrorForAMI_SuperIO		SIO_BIO->IOSetBit(0x0A, 0xE4, BIT4);	//Enable 3VSBSW
	IOSetBit(0x0A, 0xE4, BIT4);
		PortRestoreACPowerLoss();	//<ASUS-Terry2_Wang140708+>
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   ProjectS4SmiHandler
//
// Description: This is a template Project Sx SMI Handler for Porting.
//
// Parameters:  DispatchHandle  - EFI Handle
//              DispatchContext - Pointer to the EFI_SMM_SX_DISPATCH_CONTEXT
//
// Returns:     None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID ProjectS4SmiHandler (
    IN EFI_HANDLE                   DispatchHandle,
    IN EFI_SMM_SX_DISPATCH_CONTEXT  *DispatchContext )
{
    // Porting if needed
	Clear8042OutputBuffer();			//ASUS-Justin-debug0104A
	//CheckPS2KeyboardWakeUp();			//ASUS-Justin-debug0104A
		PortRestoreACPowerLoss();	//<ASUS-Terry2_Wang140708+>
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   ProjectS5SMIporting
//
// Description: This is a template Project Sx SMI Handler for Porting.
//
// Parameters:  DispatchHandle  - EFI Handle
//              DispatchContext - Pointer to the EFI_SMM_SX_DISPATCH_CONTEXT
//
// Returns:     None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID	ProjectS5SMIporting()
{
    // Porting if needed
	UINT8	Data, Index;	//<ASUS-DTL20110531>
	UINT16	Value, keycodes=0;	//<ASUS-DTL20110531>

	Clear8042OutputBuffer();

	//Check PS2 keyboard power up
//lmrtestFixMakeErrorForAMI_SuperIO	SIO_BIO->SIOBasicIORead(0x0A, 0xE3, &Data);	//ClearStatus
//lmrtestFixMakeErrorForAMI_SuperIO	SIO_BIO->IOResetBit(0x0A, 0xE0, BIT6);
//lmrtestFixMakeErrorForAMI_SuperIO	if(SetupDataPtr->PowerupKB){
//lmrtestFixMakeErrorForAMI_SuperIO		SIO_BIO->IOSetBit(0x0A, 0xE0, (BIT6+BIT0));
//lmrtestFixMakeErrorForAMI_SuperIO		SIO_BIO->IOSetBit(0x0A, 0xE6, BIT7);
//lmrtestFixMakeErrorForAMI_SuperIO	}

//lmrtestFixMakeErrorForAMI_Super_IO 	if(SetupDataPtr->PowerupMS){
//lmrtestFixMakeErrorForAMI_Super_IO //<ASUS-LMR111201+> patch PS/2 mouse failed to power on under DOS due to CSM module reset PS/2 mouse at PrepareToBoot_FAR
//lmrtestFixMakeErrorForAMI_Super_IO 		if (IoRead8(0x64)!=0xff){	//make sure 64h port is decoded
//lmrtestFixMakeErrorForAMI_Super_IO 			//force to enable PS/2 mouse
//lmrtestFixMakeErrorForAMI_Super_IO 			IoWrite8(0x64,0xD4);		//write to auxiliary device
//lmrtestFixMakeErrorForAMI_Super_IO 			//waiting for I/O port 64h[1]=0
//lmrtestFixMakeErrorForAMI_Super_IO 			for (Value=0;Value<1000;Value++){
//lmrtestFixMakeErrorForAMI_Super_IO 				if(!(IoRead8(0x64) & 0x02)) break;
//lmrtestFixMakeErrorForAMI_Super_IO 				Io_Delay(1);
//lmrtestFixMakeErrorForAMI_Super_IO 			} //end for
//lmrtestFixMakeErrorForAMI_Super_IO 			IoWrite8(0x60,0xF4);		//ENABLE COMMAND
//lmrtestFixMakeErrorForAMI_Super_IO 			//waiting for I/O port 64h[1]=0
//lmrtestFixMakeErrorForAMI_Super_IO 			for (Value=0;Value<1000;Value++){
//lmrtestFixMakeErrorForAMI_Super_IO 				if(!(IoRead8(0x64) & 0x02)) break;
//lmrtestFixMakeErrorForAMI_Super_IO 				Io_Delay(1);
//lmrtestFixMakeErrorForAMI_Super_IO 			} //end for
//lmrtestFixMakeErrorForAMI_Super_IO 			//waiting for I/O port 64h[0]=1
//lmrtestFixMakeErrorForAMI_Super_IO 			for (Value=0;Value<1000;Value++){
//lmrtestFixMakeErrorForAMI_Super_IO 				if(IoRead8(0x64) & 0x01) break;
//lmrtestFixMakeErrorForAMI_Super_IO 				Io_Delay(1);
//lmrtestFixMakeErrorForAMI_Super_IO 			} //end for
//lmrtestFixMakeErrorForAMI_Super_IO 			IoRead8(0x60);			//read dummy data
//lmrtestFixMakeErrorForAMI_Super_IO 		} //end if
//lmrtestFixMakeErrorForAMI_Super_IO //<ASUS-LMR111201->
//lmrtestFixMakeErrorForAMI_Super_IO 		SIO_BIO->IOSetBit(0x0A, 0xE0, (BIT5+BIT1));
//lmrtestFixMakeErrorForAMI_Super_IO 	} else {
//lmrtestFixMakeErrorForAMI_Super_IO 		SIO_BIO->IOResetBit(0x0A, 0xE0, BIT5);
//lmrtestFixMakeErrorForAMI_Super_IO 	} //end if else

	SIOBasicIORead(0x0A, 0xE3, &Data);			//ClearStatus
	IOResetBit(0x0A, 0xE0, BIT6);
	if(SetupDataPtr->PowerupKB){
		//SIO_BIO->IOSetBit(0x0A, 0xE0, (BIT6+BIT0));
		if(SetupDataPtr->PowerupKB == 1)	keycodes=0x0029;	//Space Bar
		if(SetupDataPtr->PowerupKB == 2)	keycodes=0x1476;	//Ctrl-Esc
		if(SetupDataPtr->PowerupKB == 3)	keycodes=0x37E0;	//Power Key
		for (Index=0; Index<=0x0e; Index++) {
			Data= (UINT8)keycodes;
			SIOBasicIOWrite(0x0A, 0xE1, Index);
			SIOBasicIOWrite(0x0A, 0xE2, Data);
			keycodes>>=8;
		}
		IOSetBit(0x0A, 0xE0, BIT6);
		IOResetBit(0x0A, 0xE0, BIT0);
	}

	IOResetBit(0x0A, 0xE0, BIT5);
	if(SetupDataPtr->PowerupMS){
		IOSetBit(0x0A, 0xE0, BIT5);
		IOSetBit(0x0A, 0xE6, BIT7);
		IOResetBit(0x0A, 0xE0, BIT1);
	}

//<ASUS-LMR100422+> Support ACPLR controlled by SuperI/O and WOL after G3 function.
#ifdef SIO_SUPPORT
 #if 0	//lmrtestFixMakeError SIO_SUPPORT
//#include <Build\Board\IO\W83677HGPEI.H>
	IoWrite8(SIO_CONFIG_INDEX, 0x87);
	IoWrite8(SIO_CONFIG_INDEX, 0x87);
	IoWrite8(SIO_CONFIG_INDEX, NCT6776F_LDN_SEL_REGISTER);
	IoWrite8(SIO_CONFIG_DATA, NCT6776F_LDN_ACPI);
	if ((SetupDataPtr->AfterG3 == 0)||(SetupDataPtr->AfterG3 == 2)){	//Off or Previous state?
		if(SetupDataPtr->PowerupPCI){	//Patch PCH GbE WOL after G3	
			IoWrite8(SIO_CONFIG_INDEX, 0xE4);
			IoWrite8(SIO_CONFIG_DATA, IoRead8(SIO_CONFIG_DATA)&(~BIT6)|BIT5); //Set RxE4h[6:5]=01b
			IoWrite8(SIO_CONFIG_INDEX, 0xE7);
			IoWrite8(SIO_CONFIG_DATA, IoRead8(SIO_CONFIG_DATA)&(~BIT4)); //Set RxE7h[4]=0
//			SET_PCI8_SB(ICH_REG_GEN_PMCON_3, 1); //set SB RxA4[0]=1, RxA4[13]=1
		    IoWrite32(0xcf8, BIT31 | (SB_BUS << 16) | (SB_DEV << 11) | (SB_FUN << 8) | (ICH_REG_GEN_PMCON_3 & 0xfc));
	        Value = IoRead16(0xcfc | (UINT8)(ICH_REG_GEN_PMCON_3 & 3));
		    Value|=BIT0+BIT13;
	        IoWrite16(0xcfc | (UINT8)(ICH_REG_GEN_PMCON_3 & 3), Value);

		} else {	//WOL disabled, ACPLR Off
			IoWrite8(SIO_CONFIG_INDEX, 0xE4);
			IoWrite8(SIO_CONFIG_DATA, IoRead8(SIO_CONFIG_DATA)&(~(BIT5+BIT6))); //Set RxE4h[6:5]=00b
			IoWrite8(SIO_CONFIG_INDEX, 0xE7);
			IoWrite8(SIO_CONFIG_DATA, IoRead8(SIO_CONFIG_DATA)|BIT4); //Set RxE7h[4]=1
//			SET_PCI8_SB(ICH_REG_GEN_PMCON_3, 1); //set SB RxA4[0]=1, RxA4[13]=0
		    IoWrite32(0xcf8, BIT31 | (SB_BUS << 16) | (SB_DEV << 11) | (SB_FUN << 8) | (ICH_REG_GEN_PMCON_3 & 0xfc));
	        Value = IoRead16(0xcfc | (UINT8)(ICH_REG_GEN_PMCON_3 & 3));
		    Value=Value|BIT0&(~BIT13);
	        IoWrite16(0xcfc | (UINT8)(ICH_REG_GEN_PMCON_3 & 3), Value);
		}; //end if else
	} else if (SetupDataPtr->AfterG3 == 1){	//ON
			IoWrite8(SIO_CONFIG_INDEX, 0xE4);
			IoWrite8(SIO_CONFIG_DATA, IoRead8(SIO_CONFIG_DATA)&(~BIT6)|BIT5); //Set RxE4h[6:5]=01b
			IoWrite8(SIO_CONFIG_INDEX, 0xE7);
			IoWrite8(SIO_CONFIG_DATA, IoRead8(SIO_CONFIG_DATA)|BIT4); //Set RxE7h[4]=1
//			SET_PCI8_SB(ICH_REG_GEN_PMCON_3, 1); //set SB RxA4[0]=0, RxA4[13]=0
		    IoWrite32(0xcf8, BIT31 | (SB_BUS << 16) | (SB_DEV << 11) | (SB_FUN << 8) | (ICH_REG_GEN_PMCON_3 & 0xfc));
	        Value = IoRead16(0xcfc | (UINT8)(ICH_REG_GEN_PMCON_3 & 3));
		    Value=Value&(~(BIT0+BIT13));
	        IoWrite16(0xcfc | (UINT8)(ICH_REG_GEN_PMCON_3 & 3), Value);
	}; //end if else
	IoWrite8(SIO_CONFIG_INDEX, SIO_CONFIG_MODE_EXIT_VALUE);
 #endif
#endif //SIO_SUPPORT

//<ASUS-LMR100422->
	if(SetupDataPtr->PowerupPCI || SetupDataPtr->PowerupPCIE)
		S5_Enable_PME();
	else
		S5_Disable_PME();

	//Check PCI power up
	if(SetupDataPtr->PowerupPCI){
		Value = IoRead16(PM_BASE_ADDRESS+ICH_IOREG_GPE0_EN);
//<ASUS-LMR100422->		Value |= BIT11;		//Enable PME
		Value |= (BIT11+BIT13);		//<ASUS-LMR100422+>Enable both PCI PME and GbE PME
		IoWrite16(PM_BASE_ADDRESS+ICH_IOREG_GPE0_EN, Value);
	}else{
		Value = IoRead16(PM_BASE_ADDRESS+ICH_IOREG_GPE0_EN);
		Value &= !BIT11;		//Disable PME
		IoWrite16(PM_BASE_ADDRESS+ICH_IOREG_GPE0_EN, Value);
	}

// [Support Power on by PCIE device] Bug#60
	//Check PCIE power up
	Value = IoRead16(PM_BASE_ADDRESS+ACPI_IOREG_PM1_EN);
	Value |= BIT14;
	//disable PCIE Wake#
	IoWrite16(PM_BASE_ADDRESS+ACPI_IOREG_PM1_EN, Value);

	//Clear Status about PCIE wake#
	IoWrite16(PM_BASE_ADDRESS+ACPI_IOREG_PM1_STS, BIT14);
	
	if(SetupDataPtr->PowerupPCIE){
		Value &= ~BIT14;
		//disable PCIE Wake#
		IoWrite16(PM_BASE_ADDRESS+ACPI_IOREG_PM1_EN, Value);
	}

// [Support Power on by Ring] Bug#60
	// DISABLE RING WAKEUP
	Value = IoRead16(PM_BASE_ADDRESS+ICH_IOREG_GPE0_EN);
	Value &= ~BIT8;
	IoWrite16(PM_BASE_ADDRESS+ICH_IOREG_GPE0_EN, Value);
	// Clear Status about #RI
	IoWrite16(PM_BASE_ADDRESS+ICH_IOREG_GPE0_STS, BIT8);
	//Check RING power up
	if(SetupDataPtr->PowerupRING){
		Value |= BIT8;
		IoWrite16(PM_BASE_ADDRESS+ICH_IOREG_GPE0_EN, Value);
	}

//elliott+ for RTC wake up
	if(SetupDataPtr->PowerupRTC)
	{
		EFI_TIME time;
		EFI_STATUS Status;
		pRS->GetTime(&time,NULL);
		if (SetupDataPtr->PowerupRTCAlarmDate==0)	//<ASUS-RIck1_Chung111013+>
			time.Day=1;
		else	//<ASUS-RIck1_Chung111013->
			time.Day=SetupDataPtr->PowerupRTCAlarmDate;
        time.Hour=SetupDataPtr->PowerupRTCHour;
        time.Minute=SetupDataPtr->PowerupRTCMinute;
        time.Second=SetupDataPtr->PowerupRTCSecond;
		Status=pRS->SetWakeupTime(TRUE,&time);
		if (!EFI_ERROR(Status))
		{
			UINT16 tmp;
			tmp=IoRead16(PM_BASE_ADDRESS + ACPI_IOREG_PM1_EN);
			tmp|=BIT10;

			IoWrite16(PM_BASE_ADDRESS + ACPI_IOREG_PM1_STS, BIT10); //<ASUS-JZL20101231test>
			IoWrite16(PM_BASE_ADDRESS + ACPI_IOREG_PM1_EN, tmp);
		}
	}
//	_asm {
//jmp	$
//	}
//+elliott
	PortRestoreACPowerLoss();	//<ASUS-Terry2_Wang140708+>

//ASUS-Normduke-test-	SIO_BIO->IOSetBit(0x0A, 0xE6, BIT4);	// ASUS-YRL+ set Power-loss Last State Flag OFF
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   ProjectS5SmiHandler
//
// Description: This is a template Project Sx SMI Handler for Porting.
//
// Parameters:  DispatchHandle  - EFI Handle
//              DispatchContext - Pointer to the EFI_SMM_SX_DISPATCH_CONTEXT
//
// Returns:     None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID ProjectSwSmiHandler (
    IN EFI_HANDLE                   DispatchHandle,
    IN EFI_SMM_SW_DISPATCH_CONTEXT  *DispatchContext )
{
	UINT16 printCode=0x00;
	unsigned char min,sec;
	IoWrite8(0x70,0x02);
	min=IoRead8(0x71);
	IoWrite8(0x70,0x00);
	sec=IoRead8(0x71);
	printCode=((UINT16)min<<8) | sec;
	IoWrite16(0x80,printCode);
	
//	ProjectS5SMIporting();
}
VOID ProjectS5cSmiHandler (
    IN EFI_HANDLE                   DispatchHandle,
    IN EFI_SMM_SX_DISPATCH_CONTEXT  *DispatchContext )
{
	UINT16 printCode=0x00;
	unsigned char min,sec;
	IoWrite8(0x70,0x02);
	min=IoRead8(0x71);
	IoWrite8(0x70,0x00);
	sec=IoRead8(0x71);
	printCode=(min<<8) | sec;
	IoWrite16(0x80,printCode);
	pBS->Stall(5000000);
//	ProjectS5SMIporting();
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   ProjectPowerButtonActivated
//
// Description: If the power button is pressed, then this function is called.
//
// Input:
//  IN EFI_HANDLE                               DispatchHandle
//  IN EFI_SMM_POWER_BUTTON_DISPATCH_CONTEXT    *DispatchContext
//
// Output: VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID ProjectPowerButtonActivated(
    IN EFI_HANDLE                               DispatchHandle,
    IN EFI_SMM_POWER_BUTTON_DISPATCH_CONTEXT    *DispatchContext
    )
{
//	ProjectS5SMIporting();
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   InSmmFunction
//
// Description: Installs CRB SMM Child Dispatcher Handler.
//
// Parameters:  ImageHandle - Image handle
//              *SystemTable - Pointer to the system table
//
// Returns:     EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS InSmmFunction (
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable )
{
	//******************   SW  SMI  Variable declaration**************************************
	EFI_GUID gSwDispatchProtocolGuid      =  EFI_SMM_SW_DISPATCH_PROTOCOL_GUID;
	EFI_SMM_SW_DISPATCH_PROTOCOL *pSwDispatch;
	EFI_SMM_SW_DISPATCH_CONTEXT  SwContext = {0xa8};
	EFI_HANDLE			Handle;
	//******************   SX  SMI  Variable declaration**************************************
	EFI_GUID gSxDispatchProtocolGuid      = EFI_SMM_SX_DISPATCH_PROTOCOL_GUID;
	EFI_SMM_SX_DISPATCH_PROTOCOL *pSxDispatch;
	EFI_SMM_SX_DISPATCH_CONTEXT  S5Context = {SxS5, SxEntry};
	//******************   SW  SMI  Achieve**************************************
	EFI_STATUS Status  = pBS->LocateProtocol( &gSwDispatchProtocolGuid, \
                                   NULL, \
                                   &pSwDispatch );

 	Status  = pSwDispatch->Register( pSwDispatch, \
                                         ProjectSwSmiHandler, \
                                         &SwContext, \
                                         &Handle );
	//******************   SX  SMI  Achieve**************************************
	Status  = pBS->LocateProtocol( &gSxDispatchProtocolGuid, \
                                   NULL, \
                                   &pSxDispatch );

 	Status  = pSxDispatch->Register( pSxDispatch, \
                                         ProjectS5cSmiHandler, \
                                         &S5Context, \
                                         &Handle );

	PreparePotocolForUse(ImageHandle, SystemTable );
	return  EFI_SUCCESS;

	/*
	EFI_STATUS                   Status;
	EFI_GUID gSxDispatchProtocolGuid      = EFI_SMM_SX_DISPATCH_PROTOCOL_GUID;
	EFI_GUID gEfiSmmPowerButtonDispatchProtocolGuid = EFI_SMM_POWER_BUTTON_DISPATCH_PROTOCOL_GUID;

	EFI_SMM_POWER_BUTTON_DISPATCH_PROTOCOL  *PowerButton;
	EFI_SMM_POWER_BUTTON_DISPATCH_CONTEXT	PBContext = {PowerButtonEntry};
	EFI_SMM_SX_DISPATCH_PROTOCOL *pSxDispatch;
	EFI_SMM_SX_DISPATCH_CONTEXT  S1Context = {SxS1, SxEntry};
	EFI_SMM_SX_DISPATCH_CONTEXT  S3Context = {SxS3, SxEntry};
	EFI_SMM_SX_DISPATCH_CONTEXT  S4Context = {SxS4, SxEntry};
	EFI_SMM_SX_DISPATCH_CONTEXT  S5Context = {SxS5, SxEntry};
	EFI_SMM_SX_DISPATCH_CONTEXT  S1ResumeContext = {SxS1, SxExit};
	EFI_HANDLE                   Handle, PBHandle;

    Status  = pBS->LocateProtocol( &gSxDispatchProtocolGuid, \
                                   NULL, \
                                   &pSxDispatch );
    if (!EFI_ERROR(Status)) {
        Status  = pSxDispatch->Register( pSxDispatch, \
                                         ProjectS1SmiHandler, \
                                         &S1Context, \
                                         &Handle );

        Status  = pSxDispatch->Register( pSxDispatch, \
                                         ProjectS3SmiHandler, \
                                         &S3Context, \
                                         &Handle );

        Status  = pSxDispatch->Register( pSxDispatch, \
                                         ProjectS4SmiHandler, \
                                         &S4Context, \
                                         &Handle );

        Status  = pSxDispatch->Register( pSxDispatch, \
                                         ProjectS5SmiHandler, \
                                         &S5Context, \
                                         &Handle );

        Status  = pSxDispatch->Register( pSxDispatch, \
                                         ProjectS1ResumeSmiHandler, \
                                         &S1ResumeContext, \
                                         &Handle );
    }

	Status = pBS->LocateProtocol( &gEfiSmmPowerButtonDispatchProtocolGuid,NULL,&PowerButton );
	if (!EFI_ERROR(Status))
		Status = PowerButton->Register( PowerButton, \
						ProjectPowerButtonActivated, \
						&PBContext, \
						&PBHandle );

	PreparePotocolForUse(ImageHandle, SystemTable );
	return EFI_SUCCESS;
	*/

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   InitializeCRBSmm
//
// Description: Installs CRB SMM Child Dispatcher Handler.
//
// Parameters:  ImageHandle - Image handle
//              *SystemTable - Pointer to the system table
//
// Returns:     EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS InitializeProjectSxSmm (
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable )
{
    InitAmiLib(ImageHandle, SystemTable);
	return InitSmmHandler(ImageHandle, SystemTable, InSmmFunction, NULL);
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2007, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**             6145-F Northbelt Pkwy, Norcross, GA 30071            **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
