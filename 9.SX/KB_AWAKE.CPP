#include<stdio.h>
#include<dos.h>
#define NCT6791D_CONFIG_INDEX 0x2E
#define NCT6791D_CONFIG_DATA 0x2F
#define PCI_CONFIG_ADDR_PORT 0x0CF8
#define PCI_CONFIG_DATA_PORT 0x0CFC
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT(i)     ( 0x01<<i ) 
#define DWORD(i)   ( 0X00000001<<i)
#define WORD(i)    ( 0x0001<<i)
typedef unsigned char UINT8;
typedef unsigned int UINT16;
typedef unsigned long long UINT32;

typedef unsigned char BOOLEAN;
void IoWrite8(UINT8 port, UINT8 data);
UINT8 IoRead8(UINT8 port);
void Turn_On_5V_DAUL_USBKB_Power();
void Turn_On_KB_Power_ON();
UINT32 inport_long(UINT16 portNum);
void outport_long(UINT16 portNum,UINT32 cmd);
UINT32 ReadPMBase();
void shutDown();
void main()
{
	char ch=0;
	Turn_On_KB_Power_ON();
	Turn_On_5V_DAUL_USBKB_Power();
	ch=getchar();
	shutDown();
	ch=ch+1;
}
void shutDown()
{
	UINT32 pmBase=0;
	UINT16 reg,yuShu;
	pmBase=ReadPMBase();
	pmBase=pmBase & 0x0000ff80;

	reg=inpw(pmBase+0x30);
	reg=reg & (~(0x0010));
	outpw(pmBase+0x30,reg);
	//S3 status
	reg=inpw(pmBase+0x04);
	reg=reg | 0x3400;
	outpw(pmBase+0x04,reg);
	/* S5 status
	reg=inpw(pmBase+0x04);
	reg=reg | 0x3c00;
	outpw(pmBase+0x04,reg);
	*/
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
	outport_long(PCI_CONFIG_ADDR_PORT,cmd);
	result=inport_long(PCI_CONFIG_DATA_PORT);
	printf("ACPI Base Address is 0x%08llx\n",result);
	return result;
}
void Turn_On_KB_Power_ON()
{
	UINT8 Reg;
	IoWrite8(NCT6791D_CONFIG_INDEX, 0x87);
	IoWrite8(NCT6791D_CONFIG_INDEX, 0x87);

	IoWrite8(NCT6791D_CONFIG_INDEX, 0x07);
	IoWrite8(NCT6791D_CONFIG_DATA,  0x0A);

	IoWrite8(NCT6791D_CONFIG_INDEX, 0x30);
	Reg = IoRead8(NCT6791D_CONFIG_DATA);
	Reg |= 0x01;					
	IoWrite8(NCT6791D_CONFIG_DATA,  Reg);


	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE0);
	Reg=IoRead8(NCT6791D_CONFIG_DATA);
	Reg=Reg |  (BIT(6) + BIT(5) + BIT(4) + BIT(1) + BIT(0));
	Reg=Reg & ~BIT(7);
	printf("LDN A index 0xE0 is %x\n",Reg);
	IoWrite8(NCT6791D_CONFIG_DATA, Reg);

	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE6);
	Reg=IoRead8(NCT6791D_CONFIG_DATA);
	Reg=Reg | BIT(7);
	printf("LDN A index 0xE6 is %x\n",Reg);
	IoWrite8(NCT6791D_CONFIG_DATA, Reg);


	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE4);
	Reg=IoRead8(NCT6791D_CONFIG_DATA);
	Reg = Reg | 0x10;
	IoWrite8(NCT6791D_CONFIG_DATA, Reg);

	//E
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE1);					
	IoWrite8(NCT6791D_CONFIG_DATA, 0x00);
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE2);
	IoWrite8(NCT6791D_CONFIG_DATA, 0x24);
	
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE1);
	IoWrite8(NCT6791D_CONFIG_DATA, 0x01);
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE2);
	IoWrite8(NCT6791D_CONFIG_DATA, 0xF0);
	
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE1);
	IoWrite8(NCT6791D_CONFIG_DATA, 0x02);
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE2);
	IoWrite8(NCT6791D_CONFIG_DATA, 0x24);
	//R
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE1);
	IoWrite8(NCT6791D_CONFIG_DATA, 0x03);
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE2);
	IoWrite8(NCT6791D_CONFIG_DATA, 0x2D);
	
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE1);
	IoWrite8(NCT6791D_CONFIG_DATA, 0x04);
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE2);
	IoWrite8(NCT6791D_CONFIG_DATA, 0xF0);
	
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE1);
	IoWrite8(NCT6791D_CONFIG_DATA, 0x05);
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE2);
	IoWrite8(NCT6791D_CONFIG_DATA, 0x2D);
	//A
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE1);
	IoWrite8(NCT6791D_CONFIG_DATA, 0x06);
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE2);
	IoWrite8(NCT6791D_CONFIG_DATA, 0x1C);
	
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE1);
	IoWrite8(NCT6791D_CONFIG_DATA, 0x07);
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE2);
	IoWrite8(NCT6791D_CONFIG_DATA, 0xF0);
	
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE1);
	IoWrite8(NCT6791D_CONFIG_DATA, 0x08);
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE2);
	IoWrite8(NCT6791D_CONFIG_DATA, 0x1C);
	//L
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE1);
	IoWrite8(NCT6791D_CONFIG_DATA, 0x09);
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE2);
	IoWrite8(NCT6791D_CONFIG_DATA, 0x4B);
	
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE1);
	IoWrite8(NCT6791D_CONFIG_DATA, 0x0A);
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE2);
	IoWrite8(NCT6791D_CONFIG_DATA, 0xF0);
	
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE1);
	IoWrite8(NCT6791D_CONFIG_DATA, 0x0B);
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE2);
	IoWrite8(NCT6791D_CONFIG_DATA, 0x4B);
	//C
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE1);
	IoWrite8(NCT6791D_CONFIG_DATA, 0x0C);
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE2);
	IoWrite8(NCT6791D_CONFIG_DATA, 0x21);
	
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE1);						
	IoWrite8(NCT6791D_CONFIG_DATA, 0x0D);
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE2);
	IoWrite8(NCT6791D_CONFIG_DATA, 0xF0);
	
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE1);						
	IoWrite8(NCT6791D_CONFIG_DATA, 0x0E);
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE2);
	IoWrite8(NCT6791D_CONFIG_DATA, 0x21);
	
	


	
	IoWrite8(NCT6791D_CONFIG_INDEX, 0xaa);
}
void IoWrite8(UINT8 port, UINT8 data)
{
	outportb(port,data);
}
UINT8 IoRead8(UINT8 port)
{
	return inportb(port);
}
void Turn_On_5V_DAUL_USBKB_Power()
{
//Switch +5V_DAUL_USBKB to standby power
	UINT8 Reg = 0;

	IoWrite8(NCT6791D_CONFIG_INDEX, 0x87);
	IoWrite8(NCT6791D_CONFIG_INDEX, 0x87);

	IoWrite8(NCT6791D_CONFIG_INDEX, 0x07);
	IoWrite8(NCT6791D_CONFIG_DATA,  0x07);

	IoWrite8(NCT6791D_CONFIG_INDEX, 0x30);
	Reg = IoRead8(NCT6791D_CONFIG_DATA);
	Reg |= BIT1;					//Active GPIO7 Group
	IoWrite8(NCT6791D_CONFIG_DATA,  Reg);

	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE0);
	Reg = IoRead8(NCT6791D_CONFIG_DATA);
	Reg &= ~BIT2;					//Set GPIO72 to output
	IoWrite8(NCT6791D_CONFIG_DATA,  Reg);	

	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE1);
	Reg = IoRead8(NCT6791D_CONFIG_DATA);
	Reg |= BIT2;					//Set GPIO72 to output High
	IoWrite8(NCT6791D_CONFIG_DATA,  Reg);

	IoWrite8(NCT6791D_CONFIG_INDEX, 0x07);
	IoWrite8(NCT6791D_CONFIG_DATA,  0x0F);	

	IoWrite8(NCT6791D_CONFIG_INDEX, 0xE6);
	Reg = IoRead8(NCT6791D_CONFIG_DATA);
	Reg &= ~BIT2;					//Set GPIO72 to Push-Pull
	IoWrite8(NCT6791D_CONFIG_DATA,  Reg);	

	IoWrite8(NCT6791D_CONFIG_INDEX, 0x1D);		//SET GP54 TO PWROK
	Reg = IoRead8(NCT6791D_CONFIG_DATA);
	Reg &= ~(BIT1+BIT2+BIT3);
	IoWrite8(NCT6791D_CONFIG_INDEX, 0x1D);
	IoWrite8(NCT6791D_CONFIG_DATA, Reg|(BIT3+BIT2));

	IoWrite8(NCT6791D_CONFIG_INDEX, 0xaa);
}
void outport_long(UINT16 portNum,UINT32 cmd)
{
       /*	ifdef Dbug
	printf("-----In the outport Function-----------\n");
	printf("this portNum is 0x%X\n",portNum);
	printf("this cmd is 0x%llx\n",cmd);
	endif*/
	_asm{
		push eax;
		push edx;
		mov dx,portNum;
		mov eax,cmd;
		out dx,eax;
		pop edx;
		pop eax;
	}
}
UINT32 inport_long(UINT16 portNum)
{
       /*	printf("--------in the inport function----------\n");
	printf("this portNum is 0x%X\n",portNum);
	*/
	UINT32 result=0;
	_asm{
		push eax;
		push edx;
		mov dx,portNum;
		in eax,dx;
		mov result,eax;
		pop edx;
		pop eax;
	}
       //	printf("this result is %llX\n",result);
	return result;
}