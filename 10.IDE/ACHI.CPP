#include<stdio.h>
#include<dos.h>
#define PCI_CONFIG_ADDR_PORT 0x0CF8
#define PCI_CONFIG_DATA_PORT 0x0CFC
#define BIT(i)     ( 0x01<<i )
#define WORD(i)     ( 0x0001<<i )
#define DWORD(i)     ( 0x00000001<<i )
#define CLBaseAddr 0x00200000
typedef unsigned long long UINT32;
typedef unsigned int UINT16;
typedef unsigned char UINT8;
void openA20();
void set4gb();
UINT32 ReadMemByPM(UINT32 addr);
void WriteMemByPM(UINT32 addr,UINT32 cReg);

void outport_long(int portNum,UINT32 cmd);
UINT32 inport_long(int portNum);

UINT32 ReadABAR();
void ReadData(UINT32 addr);
void clearMem()
{
	printf("begin to clean mem\n");
	UINT32 bAddr=0x00200000,eAddr=0x00220000;
	while(bAddr<eAddr)
	{
		WriteMemByPM(bAddr,0);
		//printf("%llx\n",bAddr);
		bAddr+=4;
	}
	return;
}
void main()
{
	printf("********************  New Game   ******************\n");
	UINT32 Reg=0;
	openA20();
	set4gb();
	Reg=ReadMemByPM(0x00200000);
	printf("reg is %08llx\n",Reg);
	//return;
	clearMem();
	UINT32 HBABaseAddress=0;
	//UINT32 Reg=0;
	UINT32 CTBaseAddr=0;

	HBABaseAddress=ReadABAR();
	printf("HBABaseAddress is %llx\n",HBABaseAddress);
	/*
	step1......................................
	set AHCI Enable
	 (set HBA Base reg offset 4h, bit31)
	*/
	Reg=ReadMemByPM(HBABaseAddress+0x04);
	printf("1 reg is %llx\n",Reg);
	Reg = Reg | 0x80000000;
	WriteMemByPM(HBABaseAddress+0x04,Reg);
	Reg=ReadMemByPM(HBABaseAddress+0x04);
	printf("2 reg is %llx\n",Reg);
	/*
step2......................................
clear status.
 
  clear port serial ata error register.
	(port reg offset 30h)

  clear port interrupt status register.
	(port reg offset 10h)

  clear base physical port interrupt status register
	(AHCI generic host control reg offset 08h)
*/
	printf("***************step2**************\n");
	WriteMemByPM(HBABaseAddress+0x100+0x30,0xFFFFFFFF);
	WriteMemByPM(HBABaseAddress+0x100+0x10,0x0);
	WriteMemByPM(HBABaseAddress+0x08,0x0);
	printf("step2 is success\n");
/*
step3......................................
	Put port in idle mode.

   Stop Port command list processing.
	(port reg offset 18h,bit0)

   Wait for controller not running.
	(port reg offset 18h,bit15)

   Disable FIS receive.
	(port reg offset 18h,bit4)

   Wait for controller recieving FIS becomes idle.
	(port reg offset 18h,bit14)
*/
	printf("***************step3.1**************\n");
	Reg = ReadMemByPM(HBABaseAddress+0x100+0x18);
	Reg = Reg & ~DWORD(0);
	WriteMemByPM(HBABaseAddress+0x100+0x18,Reg);

	Reg = ReadMemByPM(HBABaseAddress+0x100+0x18);
	Reg = Reg & DWORD(15);
	while(Reg)
	{
		Reg = ReadMemByPM(HBABaseAddress+0x100+0x18);
		Reg = Reg & DWORD(15);
	}
	printf("***************step3.2**************\n");
	Reg = ReadMemByPM(HBABaseAddress+0x100+0x18);
	Reg = Reg & ~DWORD(4);
	WriteMemByPM(HBABaseAddress+0x100+0x18,Reg);

	Reg = ReadMemByPM(HBABaseAddress+0x100+0x18);
	Reg = Reg & DWORD(14);
	while(Reg)
	{
		Reg = ReadMemByPM(HBABaseAddress+0x100+0x18);
		Reg = Reg & DWORD(14);
	}
	printf("Step 3 success!\n");
	/*
	step4......................................
	reset port
	    Send port reset signal.
		(port reg offset 2ch,bit0-bit3) = set to 1

	    Wait for 5ms

	   stop port reset signal.
		(port reg offset 2ch,bit0-bit3) = set to 0
	*/
	Reg = ReadMemByPM(HBABaseAddress+0x100+0x2C);
	Reg = Reg | DWORD(3) | DWORD(2) | DWORD(1) | DWORD(0) ;
	WriteMemByPM(HBABaseAddress+0x100+0x2C,Reg);
	delay(5);
	Reg = ReadMemByPM(HBABaseAddress+0x100+0x2C);
	Reg = Reg & ~DWORD(3) & ~DWORD(2) & ~DWORD(1) & ~DWORD(0) ;
	WriteMemByPM(HBABaseAddress+0x100+0x2C,Reg);
	printf("Step 4 success!\n");

	/*
step5......................................
Wait for device detect and communication establish.

   check port serial ata status register.
	(port reg offset 28h,bit0-bit3) = 3
*/
	UINT32 cnt=500;
	printf("****************step5**************\n");
	Reg = ReadMemByPM(HBABaseAddress+0x100+0x28);
	printf("reg is %llx\n",Reg);
	Reg = Reg & (DWORD(3) | DWORD(2) | DWORD(1) | DWORD(0));

	while(Reg!=0x03)
	{
		Reg = ReadMemByPM(HBABaseAddress+0x100+0x28);
		Reg = Reg & (DWORD(3) | DWORD(2) | DWORD(1) | DWORD(0));
		printf("cnt is %llx\n",cnt);
		cnt--;
		if(cnt==0)
			break;
	}
	printf("Step 5 success!\n");
//        return;
	/*
step6......................................
Clear port serial ata error register.
	(port reg offset 30h)
*/
	WriteMemByPM(HBABaseAddress+0x100+0x30,0xFFFFFFFF);
	printf("Step 6 success!\n");
/*
step7......................................
Reset port successful , then check the Signature of device
	(port reg offset 24h,bit31:16)  = 0000h ATA device Signature
					= EB14h ATAPI device Signature
					= 9669h	Port Multiplier Signature
*/
	Reg = ReadMemByPM(HBABaseAddress+0x100+0x24) & 0xffff0000;
	Reg = Reg >> 16;
	printf("Signature of device is %llx\n",Reg);
	printf("Step 7 success!\n");
/*
step9......................................
Set command list base
Set the command list base address in port reg (offset 00h)PxCLB ,(offset 04h)PxCLBU
*/
	Reg = ReadMemByPM(HBABaseAddress+0x100);
	Reg = Reg & 0x000003FF;
	Reg = Reg | CLBaseAddr;
	printf("Reg is %llx\n",Reg);
	WriteMemByPM(HBABaseAddress+0x100,Reg);
	WriteMemByPM(HBABaseAddress+0x100+0x04,0);
	printf("Step 9 success!\n");
/*

step10......................................
Set command table base
Set the command table base address in Command Header0 DW2(low addr),DW3(high addr) of Command list.(CLBas+08h,CLBas+0Ch)
*/
	CTBaseAddr = CLBaseAddr + 0x400;
	printf("CTBaseAddr is %llx\n",CTBaseAddr);
	Reg = ReadMemByPM(CLBaseAddr + 0x08);
	printf("Reg is %llx\n",Reg);
	Reg = Reg & 0x0000007F;
	Reg =Reg | CTBaseAddr;
	printf("Reg is %llx\n",Reg);
	WriteMemByPM(CLBaseAddr + 0x08,Reg);
	WriteMemByPM(CLBaseAddr + 0x0C,0x00);
	printf("Step 10 success!\n");
/*

step11......................................
Build command table
	Fill H2D Command FIS 
	Command tablebase+ 00h, DW0  ----- byte2 ATA Command (Ex:20h is the Read Sector Command);how to set C bit.
			   04h,	DW1  ----- device, LBA
			   08h, DW2  ----- 
			   0Ch, DW3  ----- Sector Count
			   10h, DW4  ----- reserved
*/
	Reg = ReadMemByPM(CTBaseAddr);
	Reg = Reg & 0x00007000;
	Reg = Reg | 0x00208027;
	printf("Step 11 Reg is %llx\n",Reg);
	WriteMemByPM(CTBaseAddr,Reg);
	WriteMemByPM(CTBaseAddr + 0x04,0xE0000000);
	WriteMemByPM(CTBaseAddr + 0x0C,0x00000001);
	WriteMemByPM(CTBaseAddr + 0x10,0x00000000);
	printf("Step 11 success!\n");
/*
step12......................................
Fill Physical region descriptor table in Command table
	Fill PRDT
	Command tablebase+ 80h, DW0  ----- This is the physical address of data block, data transmited or received by HBA all store here.
			   84h,	DW1 = 00000000h ----- Data block upper address.
			   88h, DW2 = 00000000h ----- reserved
			   8Ch, DW3 = 000001FFh ----- Bit31=0,indicate don't assert INT when data has transferred;1FFh indicate there are 200h bytes in data block .
*/
	WriteMemByPM(CTBaseAddr + 0x80,CTBaseAddr + 0x200);
	WriteMemByPM(CTBaseAddr + 0x8C,0x000001FF);
	printf("Step 12 success!\n");
/*


step13......................................
Fill the Command Header0 .(set PRDTL, CFL)
*/
	WriteMemByPM(CLBaseAddr + 0x00,0x00010005);
/*
step14......................................
Set the received FIS base address.
	(port reg offset 08h) PxFB ----- This is the physical address of FIS .
	(port reg offset 0Ch) PxFBU= 00000000h ----- FIS upper address.
*/
	UINT32 FISBaseAddr = CLBaseAddr + 0x10000;
	WriteMemByPM(HBABaseAddress+0x100+0x08,FISBaseAddr);
	WriteMemByPM(HBABaseAddress+0x100+0x0C,0x0);
/*

step15......................................
Enable FIS receive and ensure FIS is running. 
	set (port reg offset 18h) PxCMD.bit4 
	Check (port reg offset 18h) PxCMD.bit14  
	
	0x0000E017
*/
	Reg = ReadMemByPM(HBABaseAddress+0x100+0x18);
	Reg = Reg | DWORD(4);
	WriteMemByPM(HBABaseAddress+0x100+0x18,Reg);
	
	Reg = ReadMemByPM(HBABaseAddress+0x100+0x18);
	Reg = Reg & DWORD(14);
	while(Reg!=DWORD(14))
	{
		Reg = ReadMemByPM(HBABaseAddress+0x100+0x18);
		Reg = Reg & DWORD(14);
	}
	printf("Step 15 success!\n");
/*

step16......................................
Ensure that HBA is ready to accept command.
	ensure (port reg offset 10h) PxIS.bit6=0   , port is not changed,otherwise clear (port reg offset 30h) PxSERR register.
	ensure (port reg offset 18h) PxCMD.bit0=0  , port is IDLE.
	ensure (port reg offset 20h) PxTFD.bit3=0, bit7=0 , interface is free.
*/
	Reg = ReadMemByPM(HBABaseAddress+0x100+0x10);
	if(Reg & DWORD(6))
	{
		WriteMemByPM(HBABaseAddress+0x100+0x30,0xFFFFFFFF);
	}
	Reg = ReadMemByPM(HBABaseAddress+0x100+0x18);
	if(Reg & DWORD(0))
	{
		printf("some error  happen in step 16 : 0port is not IDLE\n");
	}
	Reg = ReadMemByPM(HBABaseAddress+0x100+0x20);
	if((Reg & DWORD(7)) | (Reg & DWORD(3)))
	{
		printf("some error  happen in step 16 : interface is not free\n");
	}
	printf("Step 16 success!\n");

/*

step17......................................
start command

	Enable Command List Process
	set (port reg offset 18h) PxCMD.bit0
	Issue command
	set (port reg offset 38h) PxCI slot corresponding bit
*/
	Reg = ReadMemByPM(HBABaseAddress+0x100+0x18);
	Reg = Reg | 0x01;
	WriteMemByPM(HBABaseAddress+0x100+0x18,Reg);
	WriteMemByPM(HBABaseAddress+0x100+0x38,0x00000001);
	printf("Step 17 success!\n");
/*
step18......................................
Wait for receive PIO setup FIS . PSFIS base = Received FIS base address + 20h
	ensure (PSFIS base + 00h), byte0 = 5Fh ----- Byte0 is the FIS type ,5Fh is the pio setup FIS .
	ensure (PSFIS base + 02h), byte2.bit3 = 1 ----- Byte2 is the FIS status ,bit3 is data ready .
	ensure (PSFIS base + 01h), byte1.bit5 = 1 ----- Value 1 indicate the data xfer direction is D2H, Value 0 indicate the direction is
	*/
	UINT32 PSFISBase = FISBaseAddr + 0x20;
	printf("PSFISBase is %llx\n",PSFISBase);
	Reg = ReadMemByPM(PSFISBase+0x00);
	printf("Reg is %llx\n",Reg);
	while(Reg & 0x800000)
	{
		if(Reg & 0x010000)
			break;
		Reg = ReadMemByPM(PSFISBase+0x00);
	}
	if(Reg & 0x010000)
	{
		printf("step18: some error happen\n");
	}
	printf("Reg is %llx\n",Reg);
	while((Reg & 0x0ff) != 0x5F)
	{
		Reg = ReadMemByPM(PSFISBase+0x00);
	}
	while((Reg & DWORD(19))!=DWORD(19))
		Reg = ReadMemByPM(PSFISBase+0x00);
	while((Reg & DWORD(13))!=DWORD(13))
		Reg = ReadMemByPM(PSFISBase+0x00);
	printf("step18 is success!\n");
	printf("addr is %llx\n",CTBaseAddr + 0x200);
	ReadData(CTBaseAddr + 0x200);

}
void ReadData(UINT32 addr)
{
	UINT32 reg=0,cnt=128,index=0;
	while(cnt--)
	{
		reg=ReadMemByPM(addr);
		printf("%8.8llx ",reg);
		index++;
		if(index%4==0)
			printf("\n");
		addr+=4;
	}

}
UINT32 ReadABAR()
{
	UINT32 busNum=0,devNum=0,funNum=0;
	UINT32 baseCmd=0x80000000,cmd,cmd_temp;
	UINT32 result=0;
	UINT32 i,j,k;
	for(i=0;i<64;i++)//for bus number
		for(j=0;j<32;j++)//for device id
			for(k=0;k<8;k++)
			{
				busNum=i;devNum=j;funNum=k;
				cmd=baseCmd|(busNum<<16)|(devNum<<11)|(funNum<<8);

				outport_long(PCI_CONFIG_ADDR_PORT,cmd);
				result=inport_long(PCI_CONFIG_DATA_PORT);
				if(result!=0xffffffff)
				{
					cmd_temp = cmd + 0x08;
					outport_long(PCI_CONFIG_ADDR_PORT,cmd_temp);
					result=inport_long(PCI_CONFIG_DATA_PORT);
					//printf("cmd + 0x08 is %llx\n",result);
					if(((result & 0xFFFF0000) == 0x01010000) || ((result & 0xFFFF0000) == 0x01060000))
					{
						cmd_temp = cmd + 0x24;
						outport_long(PCI_CONFIG_ADDR_PORT,cmd_temp);
						result=inport_long(PCI_CONFIG_DATA_PORT);
						return result;// & 0xFFFFE000;
					}
				}
			}
	printf("cmd + 0x08 is %llx\n",result);
	return 0;
}
void outport_long(int portNum,UINT32 cmd)
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
UINT32 inport_long(int portNum)
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

void WriteMemByPM(UINT32 addr,UINT32 cReg)
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
UINT32 ReadMemByPM(UINT32 addr)
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