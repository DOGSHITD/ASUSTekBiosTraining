#include <stdio.h>
//#include <conio.h>
#include <dos.h>
//#include <malloc.h>
//#include <mem.h>
//#include "MEM_Acc.h"
//#include "dmem.h"

#define PCI_CONF_ADDR_PORT 0xCF8
#define PCI_CONF_DATA_PORT 0xCFC
#define OUT 
#define IN
#define BIT(i) (0x01<<i)
#define WORD(i) (0x0001<<i)
#define DWORD(i) (0x00000001UL<<i)

typedef unsigned char UINT8;
typedef unsigned int UINT16;
typedef unsigned long long UINT32;

void outport_long(UINT16 portNum,UINT32 cmd);
UINT32 inport_long(UINT16 portNum);
void GetCapabilityBase(OUT UINT32 *CapBase,OUT UINT32 *Cnt);
void XHCI_controller_Stop(UINT32 OpBase);
void XHCI_controller_Run(UINT32 OpBase);
void XHCI_controller_Reset(UINT32 OpBase);
void displaySpeed(UINT32 OpBase,UINT32 MaxPorts);
void openA20()
{ while(inp(0x64) & 2); outp(0x64,0xd1);
while(inp(0x64) & 2); outp(0x60,0xdf);
while(inp(0x64) & 2); outp(0x64,0xff);
}
unsigned long GDT_def[]={0,0,0x0000FFFF,0x008F9200}; 
unsigned char GDT_Addr[6]={0};
void set4gb() 
{ asm{
	cli
	push ds ; push es
	mov word ptr GDT_Addr[0], (2*8-1)
	mov eax,ds
	shl eax,4
	xor ebx,ebx
	mov bx,offset GDT_def
	add eax,ebx
	mov dword ptr GDT_Addr[2],eax
	lgdt fword ptr GDT_Addr 
	mov bx,8 
	mov eax,cr0
	or al,1
	mov cr0,eax
	jmp flush1
	}
	flush1: asm{
	mov ds,bx
	mov es,bx
	and al,0feh
	mov cr0,eax
	jmp flush2
	}
	flush2: asm{
	pop es ; pop ds
	sti
	}
}
void Write_Mem_Byte(UINT32 addr,UINT8 cReg)
{
    _asm{
	push ds;
	push eax;
	mov ax,0;
	mov ds,ax;
	mov al,cReg
	mov esi,addr;
	mov [esi],al;
	pop eax;
	pop ds;
    }
}
UINT8 Read_Mem_Byte(UINT32 addr)
{
    UINT8 result;
    _asm{
	push ds;
	push eax;
	mov ax,0;
	mov ds,ax;
	mov esi,addr;
	mov al,[esi];
	mov result,al;
	pop eax;
	pop ds;
    }
    return result;
}
void Write_Mem_Dword(UINT32 addr,UINT32 cReg)
{
    _asm{
	push ds;
	push eax;
	mov ax,0;
	mov ds,ax;
	mov eax,cReg
	mov esi,addr;
	mov [esi],eax;
	pop eax;
	pop ds;
    }
}
UINT32 Read_Mem_Dword(UINT32 addr)
{
    UINT32 result;
    _asm{
	push ds;
	push eax;
	mov ax,0;
	mov ds,ax;
	mov esi,addr;
	mov eax,[esi];
	mov result,eax;
	pop eax;
	pop ds;
    }
    return result;
}
int main()
{
	printf("1--------------------------------\n");
	openA20();
	set4gb();
	//Enter_Big_Real_Mode();
	//dminstall(0x500000);
	//return 0;
	printf("2--------------------------------\n");
	UINT32 CapBaseArray[10],i,cnt,CapBase,OpBase;
	UINT8 MaxSlots,MaxPorts;
	GetCapabilityBase(CapBaseArray,&cnt);
	printf("cnt is %d\n",cnt);
	for(i=0;i<cnt;i++)
	{
		CapBase=CapBaseArray[i];
		OpBase=CapBase+Read_Mem_Byte(CapBase);
		printf("Capability Base:%08llx\nOperational Base:%08llx\n",CapBase,OpBase);
		XHCI_controller_Stop(OpBase);
		XHCI_controller_Reset(OpBase);
		MaxSlots=Read_Mem_Byte(CapBase+0x04);
		MaxPorts=Read_Mem_Byte(CapBase+0x04+0x03);
		printf("MaxSlots:%d\nMaxPorts:%d\n",MaxSlots,MaxPorts);
		Write_Mem_Byte(OpBase+0x38,MaxSlots);
		XHCI_controller_Run(OpBase);
		displaySpeed(OpBase,MaxSlots);
	}
	//Exit_Big_Real_Mode();
	return 0;
}
void displaySpeed(UINT32 OpBase,UINT32 MaxSlots)
{
	UINT32 i=0,reg,reg8;
	for(i=0;i<MaxSlots;i++)
	{
		reg=Read_Mem_Dword(OpBase+0x400+0x10*i);
		if(reg & DWORD(9))
		{
		/*	printf("reg is %08llx\n",reg);
			if((reg & 0x01E0) == 0x0E0)
			{
				printf("usb3.0 is resetting!\n");
				while((reg & 0x01E0) != 0x0)
				{
					reg=Read_Mem_Dword(OpBase+0x400+0x10*i);
					printf("%08llx \t %llx\n",reg,reg & 0x01E0);
					delay(1000);
				}
			}
		*/
			if(reg & DWORD(0))
			{
				if((reg & DWORD(1)) != DWORD(1))
				{
					reg = reg | DWORD(4);
					Write_Mem_Dword(OpBase+0x400+0x10*i,reg);
					reg=Read_Mem_Dword(OpBase+0x400+0x10*i);
					while((reg & DWORD(1)) !=DWORD(1))
					{
						reg=Read_Mem_Dword(OpBase+0x400+0x10*i);
					}
				}
				reg8=Read_Mem_Byte(OpBase+0x400+0x10*i+0x1);
				reg8=reg8 & 0x3C;
				reg8=reg8 >> 2;
				printf("USB Device Connected.");
				switch(reg8)
				{
					case 1:printf("speed full\n");break;
					case 2:printf("speed low\n");break;
					case 3:printf("speed high\n");break;
					case 4:printf("speed super\n");break;
					case 5:printf("speed super plus\n");break;
					default:printf("reserve speed\n");break;
				}
			}
			else
			{
				printf("No USB Device Connected\n");
			}
		}
	}
}
void XHCI_controller_Run(UINT32 OpBase)
{
	UINT8 reg=0;
	reg=Read_Mem_Byte(OpBase);
	reg=reg | BIT(0);
	Write_Mem_Byte(OpBase,reg);
	
	reg=Read_Mem_Byte(OpBase+0x04);
	while(reg & BIT(0))
		reg=Read_Mem_Byte(OpBase+0x04);
	delay(10);
	//printf("fe900430 %08llx\n",Read_Mem_Dword(0xfe900430ul));
	//delay(100);
	//printf("fe900430 %08llx\n",Read_Mem_Dword(0xfe900430ul));
}
void XHCI_controller_Stop(UINT32 OpBase)
{
	UINT8 reg=0;
	reg=Read_Mem_Byte(OpBase);
	reg=reg & ~BIT(0);
	Write_Mem_Byte(OpBase,reg);
	
	reg=Read_Mem_Byte(OpBase+0x04);
	while((reg & BIT(0)) != 0x01)
		reg=Read_Mem_Byte(OpBase+0x04);
}
void XHCI_controller_Reset(UINT32 OpBase)
{
	UINT8 reg=0;
	reg=Read_Mem_Byte(OpBase);
	reg=reg | BIT(1);
	Write_Mem_Byte(OpBase,reg);
	
	reg=Read_Mem_Byte(OpBase);
	while(reg & BIT(1))
		reg=Read_Mem_Byte(OpBase);
}
void GetCapabilityBase(OUT UINT32 *CapBase,OUT UINT32 *Cnt)
{
	*Cnt=0;
	UINT32 bus,dev,fun;
	UINT32 baseAddr=0x80000000,cmd,cmd_temp;
	UINT32 reg=0;
	for(bus=0;bus<32;bus++)
		for(dev=0;dev<32;dev++)
			for(fun=0;fun<8;fun++)
			{
				//bus=0;
				//dev=20;
				//fun=0;
				cmd=baseAddr | (bus<<16) | (dev<<11) | (fun<<8);
				//printf("cmd is %08llx\n",cmd);
				outport_long(PCI_CONF_ADDR_PORT,cmd);
				reg=inport_long(PCI_CONF_DATA_PORT);
				//printf("%llx\t %llx\t %llx\t %08llx\n",bus,dev,fun,reg);
				if(reg != 0xFFFFFFFF)
				{
					//printf("%d\t %d\t %d\n",bus,dev,fun);
					cmd_temp=cmd | 0x08;
					outport_long(PCI_CONF_ADDR_PORT,cmd_temp);
					reg=inport_long(PCI_CONF_DATA_PORT);
					if((reg & 0xFFFFFF00) ==0x0C033000)
					{
						cmd_temp=cmd | 0x10;
						outport_long(PCI_CONF_ADDR_PORT,cmd_temp);
						reg=inport_long(PCI_CONF_DATA_PORT);
						reg=reg & 0xFFFFFFF0;
						*(CapBase + *Cnt)=reg;
						(*Cnt)++;
					}
				}
			}
}
void outport_long(UINT16 portNum,UINT32 cmd)
{
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
	return result;
}