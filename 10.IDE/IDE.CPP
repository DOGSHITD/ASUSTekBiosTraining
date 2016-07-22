#include<stdio.h>
#include<dos.h>
#define PCI_CONFIG_ADDR_PORT 0x0CF8
#define PCI_CONFIG_DATA_PORT 0x0CFC
#define BIT(i)     ( 0x01<<i )

typedef unsigned long long UINT32;
typedef unsigned int UINT16;
typedef unsigned char UINT8;

void outport_long(int portNum,UINT32 cmd);
UINT32 inport_long(int portNum);
void InitBaseAddr(UINT16 *controlAddr,UINT16 *commandAddr);
void Native_Mode_Identify(UINT16 controlAddr,UINT16 commandAddr);
void ReadIdentify_EC(UINT16 controlAddr,UINT16 commandAddr);
void ReadIdentify_Data(UINT16 controlAddr,UINT16 commandAddr);

void main()
{
	UINT16 controlAddr=0,commandAddr=0;
	InitBaseAddr(&controlAddr,&commandAddr);
	printf("controlAddr is %x\n",controlAddr);
	printf("commandAddr is %x\n",commandAddr);
	getchar();
	Native_Mode_Identify(controlAddr,commandAddr);
	getchar();
	ReadIdentify_EC(controlAddr,commandAddr);
	getchar();
	ReadIdentify_Data(controlAddr,commandAddr);
}
/*
o 3f6 04 ; Channel Reset
o 3f6 00
Command Block:(Read Sector Command)
i 1f7 	; 50 – HD exist
o 1f2 1 	; Read 1 sector
o 1f3 0 	; Set Read Start Address
o 1f4 0
o 1f5 0
o 1f6 e0 ; Select LBA Mode
o 1f7 20 ; Send Read Command
Status Port:
i 1f7 	; 58 – HD Data Ready
*/
void ReadIdentify_EC(UINT16 controlAddr,UINT16 commandAddr)
{

	printf("**********  EC   *************\n");
	UINT8 reg=0,ret;//,flag=0;
	UINT32 cnt=1;
	UINT32 totalSize=0;
	UINT16 pos=0;
	int indexSN=0,indexSize=0,indexSize48=0;
	unsigned char snArr[20],sizeArr[4],size48Arr[16];
	outportb(controlAddr+0x02,0x04);
	outportb(0xeb,0x04);			//for delay 400ns
	outportb(controlAddr+0x02,0x00);
	reg=inportb(commandAddr+0x07);
	while(!(reg & BIT(0)) && (reg & BIT(7)))
	{
		reg=inportb(commandAddr+0x07);
	}
	if((reg & BIT(0)))
	{
		printf("ReadIdentify_EC some error happen\n");
		return;
	}
	if(!(reg & BIT(6)))
	{
		printf("ReadIdentify_EC Not ATA Drive\n");
		return;
	}
	/*
	outportb(commandAddr+0x02,0x01);
	outportb(commandAddr+0x04,0x00);
	outportb(commandAddr+0x05,0x00);
	*/
	outportb(commandAddr+0x06,0x00);
	outportb(commandAddr+0x07,0xEC);
	reg=inportb(commandAddr+0x07);
	while(!(reg & BIT(0)) && (reg & BIT(7)))
	{
		reg=inportb(commandAddr+0x07);
	}
	if((reg & BIT(0)))
	{
		printf("ReadIdentify_EC some error happen\n");
		return;
	}
	if(!(reg & BIT(3)))
	{
		printf("ReadIdentify_EC DRQ error\n");
		return;
	}
	while(!(reg & BIT(0)) && (reg & BIT(3)))
	{
		ret=inportb(commandAddr+0x00);
		reg=inportb(commandAddr+0x07);
		printf("%2.2x  ",ret);
		if(cnt%16==0)
			printf("\n");
		cnt++;
		if(pos>=20 && pos <=39)
		{
			snArr[indexSN++]=ret;
		}
		if(pos>=120 && pos <=123)
		{
			sizeArr[indexSize++]=ret;
		}
		if(pos>=200 && pos <=207)
		{
			size48Arr[indexSize48++]=ret;
		}
		pos++;
	}
	if(reg & BIT(0))
	{
		printf("ReadIdentify_EC some error happen\n");
		return;
	}
	else
		printf("success\n");
	indexSN--;
	printf("SN:");
	pos=0;
	while(pos<indexSN)
	{
		if(snArr[pos]!=0x20)
		    printf("%c%c",snArr[pos+1],snArr[pos]);
		pos+=2;
	}
	printf("\n");
	/*
	indexSize--;
	printf("Total Size:");
	while(indexSize>=0)
	{
		totalSize = totalSize | sizeArr[indexSize];
		printf("Total Size:%llx\n",totalSize);
		if(indexSize!=0)
		    totalSize = totalSize<<8;
		indexSize--;
	}
	printf("Total Size:%llx\n",totalSize);
	totalSize=totalSize/2/1024/1024;
	printf("\n");
	printf("Total Size:%dGB\n",totalSize);
	*/
	indexSize48--;
	totalSize=0;
	while(indexSize48>=0)
	{
		totalSize = totalSize | size48Arr[indexSize48];
		//printf("Total Size:%llx\n",totalSize);
		if(indexSize48!=0)
		    totalSize = totalSize<<8;
		indexSize48--;
	}
	//printf("Total Size:%llx\n",totalSize);
	totalSize=totalSize/2/1024/1024;
	printf("\n");
	printf("Total Size:%dGB\n",totalSize);
	printf("success\n");
}
void ReadIdentify_Data(UINT16 controlAddr,UINT16 commandAddr)
{
	printf("**********  DATA   *************\n");
	UINT8 reg=0,ret;
	UINT32 cnt=1;
	outportb(controlAddr+0x02,0x04);
	outportb(0xeb,0x04);			//for delay 400ns
	outportb(controlAddr+0x02,0x00);
	reg=inportb(commandAddr+0x07);
	printf("1 reg is %x\n",reg);
	while(!(reg & BIT(0)) && (reg & BIT(7)))
	{
		reg=inportb(commandAddr+0x07);
		printf("1 reg is %x\n",reg);
	}
	if((reg & BIT(0)))
	{
		printf("ReadIdentify_EC some error happen\n");
		return;
	}
	if(!(reg & BIT(6)))
	{
		printf("ReadIdentify_EC Not ATA Drive\n");
		return;
	}
	outportb(commandAddr+0x02,0x01);
	outportb(commandAddr+0x03,0x00);
	outportb(commandAddr+0x04,0x00);
	outportb(commandAddr+0x05,0x00);
	outportb(commandAddr+0x06,0xE0);
	outportb(commandAddr+0x07,0x20);
	reg=inportb(commandAddr+0x07);
	printf("2 reg is %x\n",reg);
	while(!(reg & BIT(0)) && (reg & BIT(7)))
	{
		reg=inportb(commandAddr+0x07);
		printf("2 reg is %x\n",reg);
	}
	if((reg & BIT(0)))
	{
		printf("ReadIdentify_EC some error happen\n");
		return;
	}
	if(!(reg & BIT(3)))
	{
		printf("ReadIdentify_EC DRQ error\n");
		return;
	}
	while(!(reg & BIT(0)) && (reg & BIT(3)))
	{
		ret=inportb(commandAddr+0x00);
		reg=inportb(commandAddr+0x07);
		printf("%2.2x  ",ret);
		if(cnt%16==0)
			printf("\n");
		cnt++;
	}
	if(reg & BIT(0))
	{
		printf("ReadIdentify_Data some error happen\n");
		return;
	}
	else
		printf("success\n");
}
void Native_Mode_Identify(UINT16 controlAddr,UINT16 commandAddr)
{
	UINT8 reg=0,ret;
	outportb(controlAddr+0x02,0x04);
	outportb(0xeb,0x04);			//for delay 400ns
	outportb(controlAddr+0x02,0x00);
	
	outportb(commandAddr+0x06,0xE0);
	reg=inportb(commandAddr+0x07);
	printf("Native_Mode_Identify\n",reg);
	while(!(reg & BIT(0))   &&  !(reg & BIT(6)))
	{
		//printf("reg is %x\n",reg);
		reg=inportb(commandAddr+0x07);
	}
	if(reg & BIT(0))
	{
		printf("Native_Mode_Identify some error happen\n");
	}
	else
		printf("Native_Mode_Identify success\n");
}
void InitBaseAddr(UINT16 *controlAddr,UINT16 *commandAddr)
{
	UINT32 busNum=0,devNum=17,funNum=0;
	UINT32 baseCmd=0x80000000,cmd;
	UINT32 result=0;
	UINT32 i,j,k;
	baseCmd=baseCmd|(busNum<<16)|(devNum<<11)|(funNum<<8);
	cmd=baseCmd+0x10;
	outport_long(PCI_CONFIG_ADDR_PORT,cmd);
	result=inport_long(PCI_CONFIG_DATA_PORT);
	*commandAddr=result&0x0000FFF8;
	cmd=baseCmd+0x14;
	outport_long(PCI_CONFIG_ADDR_PORT,cmd);
	result=inport_long(PCI_CONFIG_DATA_PORT);
	*controlAddr=result&0x0000FFFC;

	cmd=baseCmd+0x08;
	outport_long(PCI_CONFIG_ADDR_PORT,cmd);
	result=inport_long(PCI_CONFIG_DATA_PORT);
	result;
	printf("r is %llx\n",result);


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
