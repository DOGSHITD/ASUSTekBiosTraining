#include <stdio.h>
#include <dos.h>

#define PCI_CONF_ADDR_PORT 0xCF8
#define PCI_CONF_DATA_PORT 0xCFC
#define OUT 
#define IN
#define BIT(i) (0x01<<i)
#define WORD(i) (0x0001<<i)
#define DWORD(i) (0x00000001UL<<i)
void PCI_Config_Space(UINT32 bus,UINT32 dev,UINT32 fun,UINT32 offset)
{
	UINT32 baseAddr=0x80000000,cmd;
	UINT32 reg=0;
	cmd=baseAddr | (bus<<16) | (dev<<11) | (fun<<8) | offset;
	outport_long(PCI_CONF_ADDR_PORT,cmd);
	reg=inport_long(PCI_CONF_DATA_PORT);
	return reg;
}
UINT8 PCI_Config_Space_Byte(UINT32 bus,UINT32 dev,UINT32 fun,UINT32 offset)
{
	return (UINT8)PCI_Config_Space(bus,dev,fun,offset);
}
UINT16 PCI_Config_Space_Word(UINT32 bus,UINT32 dev,UINT32 fun,UINT32 offset)
{
	return (UINT16)PCI_Config_Space(bus,dev,fun,offset);
}
UINT16 PCI_Config_Space_Dword(UINT32 bus,UINT32 dev,UINT32 fun,UINT32 offset)
{
	return (UINT32)PCI_Config_Space(bus,dev,fun,offset);
}
void main()
{
	UINT16 reg16=0;
	reg = PCI_Config_Space_Word(0,31,0,82);
	printf("LPC_EN : %04x\n",reg);
	printf("Microcontroller Enable : %d\n",(reg&WORD(13))==WORD(13)?1:0);
	
}

