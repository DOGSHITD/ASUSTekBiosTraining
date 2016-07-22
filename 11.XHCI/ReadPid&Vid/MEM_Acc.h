#include"TYPE.h"
//**************************************************************
//	name:	Enter_Big_Real_Mode
//	input:	none
//	output:	none
//	note:							 
//**************************************************************
void Enter_Big_Real_Mode(void);

//**************************************************************
//	name:	Exit_Big_Real_Mode
//	input:	none
//	output:	none
//	note:							 
//**************************************************************
void Exit_Big_Real_Mode(void);

//**************************************************************
//	name:	Read_Mem_Byte
//	input:	adr	(Memory address)
//	output:	return(Byte data by uint8)
//	note:							 
//**************************************************************
uint8 Read_Mem_Byte(uint32 adr);

//**************************************************************
//	name:	Write_Mem_Byte
//	input:	adr	(Memory address)
//			val_b	(byte data for write)
//	output:	none
//	note:							 
//**************************************************************
void Write_Mem_Byte(uint32 adr,uint8 val_b);

//**************************************************************
//	name:	Read_Mem_Word
//	input:	adr	(Memory address)
//	output:	return(Word data by uint16)
//	note:							 
//**************************************************************
uint16 Read_Mem_Word(uint32 adr);

//**************************************************************
//	name:	Write_Mem_Word
//	input:	adr	(Memory address)
//			val_w	(Word data for write)
//	output:	none
//	note:							 
//**************************************************************
void Write_Mem_Word(uint32 adr,uint16 val_w);

//**************************************************************
//	name:	Read_Mem_Dword
//	input:	adr	(Memory address)
//	output:	return(Dword data by uint32)
//	note:							 
//**************************************************************
uint32 Read_Mem_Dword(uint32 adr);

//**************************************************************
//	name:	Write_Mem_Dword
//	input:	adr	(Memory address)
//			val_d	(Dword data for write)
//	output:	none
//	note:							 
//**************************************************************
void Write_Mem_Dword(uint32 adr,uint32 val_d);