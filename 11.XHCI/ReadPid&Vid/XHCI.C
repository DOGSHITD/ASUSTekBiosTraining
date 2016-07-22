#include <stdio.h> 
#include <conio.h> 
#include <dos.h> 
#include <malloc.h> 
#include <mem.h> 
#include "MEM_Acc.h" 
#include "dmem.h"
#define PCI_CONFIG_ADDR_PORT 0x0CF8
#define PCI_CONFIG_DATA_PORT 0x0CFC
#define BIT(i)     ( 0x01<<i )
#define WORD(i)     ( 0x0001<<i )
#define DWORD(i)     ( 0x00000001UL<<i )
#define ArraySize 10
typedef unsigned long long UINT32;
typedef unsigned int UINT16;
typedef unsigned char UINT8;

void outport_long(int portNum,UINT32 cmd);
UINT32 inport_long(int portNum);
void ReadCAPBase(UINT32 *CapBase,UINT32 *Cnt);
void ReadOpBase(UINT32 *CapBaseArray,UINT32 *OpBaseArray,UINT32 Cnt);
void XHCI_controller_Stop(UINT32);
void XHCI_controller_Reset(UINT32);
UINT8 GetMaxSlotsByCap(UINT32 CapBase);
UINT8 GetMaxPortsByCap(UINT32 CapBase);
void SetMaxSlots(UINT32 OpBase,UINT8 MaxSlots);
UINT32 DCBAAConfig(UINT32 CapBase,UINT32 OpBase,UINT32 MaxSlots);
UINT32 EventRingConfig(UINT32 CapBase,UINT32 RuntimeBase);
UINT32 CommandRingConfig(UINT32 OpBase);
void XHCI_controller_Run(UINT32 OpBase);
UINT8 DisplaySpeed(UINT32 OpBase,UINT32 MaxSlots,UINT32 *pSpeed);
UINT8 readSlot(UINT32 RuntimeBase,UINT32 eventDequeue,UINT32 *pIndex,UINT32 *pFlag);
void iSlotConfig(UINT32 InputContext,UINT32 Speed,UINT32 Maxports,UINT32 connectPort);
UINT32 InputContextConfig(UINT32 CapBase);
void iEPConfig(UINT32 InputContext,UINT32 Speed,UINT32 tDequeue);
void AddDeviceCommand(UINT32 CommandRing,UINT32 eIndex,UINT32 InputContext,UINT32 slotNum);
UINT8 checkAddress(UINT32 RuntimeBase,UINT32 eventDequeue,UINT32 *pIndex,UINT32 *pFlag,UINT32 slotNum);
UINT32 transferRingInit();
UINT32 transferRingConfig(UINT32 TransferRing);
UINT8 checkGetDescriptor(UINT32 RuntimeBase,UINT32 eventDequeue,UINT32 *pIndex,UINT32 *pFlag,UINT32 slotNum);
void ShowDescriptor(UINT32 bufAddr,UINT32 bufSize);
void commandEnqueueAdd(UINT32 *pIndex,UINT32 *pFlag);
int main()
{
	Enter_Big_Real_Mode();
	dminstall(0x500000);
	UINT32 CapBaseArray[ArraySize],OpBaseArray[ArraySize],cnt=0,i,j,index=0;
	UINT32 CapBase,OpBase;
	UINT32 dcbaa;
	UINT32 CommandRing,IRS0,TransferRing,InputContext=0,DataBuffer;
	UINT8 MaxSlots=0,Maxports=0,slotNum=0,connectPort=0;
	UINT32 reg1,reg2;
	UINT32 commandEnqueue=0,eIndex=0,eFlag=1,eventDequeue=0,dIndex=0,dFlag=1,Speed=0x55aa;
	UINT32 DoorbellBase=0x0,RuntimeBase=0;
	UINT32 iSlotContext=0;
	UINT32 iEPContext=0;
	ReadCAPBase(CapBaseArray,&cnt);						//Get capability register base address.
	ReadOpBase(CapBaseArray,OpBaseArray,cnt);			//Get operational register base address
	for(i=0;i<cnt;i++)
	{
		CapBase=CapBaseArray[i];
		OpBase=OpBaseArray[i];
		printf("CapBase is %llx,",CapBase);
		printf("OpBase is %llx\n",OpBase);
		XHCI_controller_Stop(OpBase);
		XHCI_controller_Reset(OpBase);

		MaxSlots=GetMaxSlotsByCap(CapBase);
		Maxports=GetMaxPortsByCap(CapBase);
		SetMaxSlots(OpBase,MaxSlots);
		printf("MaxSlots is %d,MaxsPorts is %d\n",MaxSlots,Maxports);

		dcbaa=DCBAAConfig(CapBase,OpBase,MaxSlots);
		CommandRing=CommandRingConfig(OpBase);				//假设已通过 opbase+0x18  = 0x000000000
		TransferRing=transferRingInit();
		RuntimeBase=CapBase + (Read_Mem_Dword(CapBase + 0x18) & 0xFFFFFFE0);
		IRS0=EventRingConfig(CapBase,RuntimeBase);

		XHCI_controller_Run(OpBase);
		connectPort=DisplaySpeed(OpBase,MaxSlots,&Speed);												//Display  Speed
		continue;
		if(connectPort==0xFF)
		{
			getchar();
			continue;
		}
		printf("main connectPort is %x\n",connectPort);
		Write_Mem_Dword(CommandRing + eIndex*0x10 + 0x0C , (0x09UL<<10) | DWORD(0));
		commandEnqueueAdd(&eIndex,&eFlag);
		DoorbellBase=CapBase+Read_Mem_Dword(CapBase+0x14);
		Write_Mem_Dword(DoorbellBase,0x0);
		
		eventDequeue=Read_Mem_Dword(IRS0+0x18) & 0xFFFFFFF0;
		/*
		printf("dcbaa is %llx\n",dcbaa);
		printf("CommandRing is %llx\n",CommandRing);
		printf("TransferRing is %llx\n",TransferRing);
		printf("RuntimeBase is %llx\n",RuntimeBase);
		printf("IRS0 is %llx\n",IRS0);
		printf("eventDequeue is %llx\n",eventDequeue);
		printf("DoorbellBase is %llx\n",DoorbellBase);
		*/
		slotNum=readSlot(RuntimeBase,eventDequeue,&dIndex,&dFlag);										//Read slot
		
		printf("slotNum is %d\n",slotNum);
		printf("speed is %08llx\n",Speed);
		InputContext=InputContextConfig(CapBase);
		iSlotConfig(InputContext,Speed,Maxports,connectPort);
		iEPConfig(InputContext,Speed,TransferRing);
		AddDeviceCommand(CommandRing,eIndex,InputContext,slotNum);
		commandEnqueueAdd(&eIndex,&eFlag);
		Write_Mem_Dword(DoorbellBase,0x0);				
		checkAddress(RuntimeBase,eventDequeue,&dIndex,&dFlag,slotNum);									//Address
		
		DataBuffer=transferRingConfig(TransferRing);
		printf("DataBuffer is %llx\n",DataBuffer);
		Write_Mem_Dword(DoorbellBase+0x4,0x01);
		checkGetDescriptor(RuntimeBase,eventDequeue,&dIndex,&dFlag,slotNum);							//Descriptor
		ShowDescriptor(DataBuffer,0x12);
		getchar();
	}
	Exit_Big_Real_Mode();
	return 0;
}
void commandEnqueueAdd(UINT32 *pIndex,UINT32 *pFlag)
{
	UINT32 dIndex=*pIndex,dFlag=*pFlag;
	dIndex++;
	if(dIndex==16)
	{
		dIndex=0;
		if(dFlag==0)
			dFlag=1;
		else if(dFlag==1)
			dFlag=0;
	}
	*pIndex=dIndex;
	*pFlag=dFlag;
}
void ShowDescriptor(UINT32 bufAddr,UINT32 bufSize)
{
	UINT16 idVendor=0,idProduct=0;
	idVendor=Read_Mem_Word(bufAddr+0x08);
	idProduct=Read_Mem_Word(bufAddr+0x0a);
	printf("Vendor ID : %04x Product ID : %04x ",idVendor,idProduct);
}
UINT8 checkGetDescriptor(UINT32 RuntimeBase,UINT32 eventDequeue,UINT32 *pIndex,UINT32 *pFlag,UINT32 slotNum)
{
	UINT32 reg=0,dIndex=*pIndex,dFlag=*pFlag,cnt=0;
	UINT8 result=0;
	reg=Read_Mem_Dword(eventDequeue+0x10*dIndex+0x0C);
	printf("reg is %llx,dIndex is %llx,dFlag is %llx\n",reg,dIndex,dFlag);
	while((reg & 0x01)!=dFlag)
	{
		reg=Read_Mem_Dword(eventDequeue+0x10*dIndex+0x0C);
	}
	printf("2.find some event TRB\n");
	while((reg & 0x01)==dFlag)
	{
		if(((reg & 0x0FC00) == 0x08000) && (Read_Mem_Byte(eventDequeue+0x10*dIndex+0x0B) == 0x01))
		{
			if(Read_Mem_Byte(eventDequeue+0x10*dIndex+0x0C + 0x03)==slotNum)
			{
				result=1;
				printf("Descriptor success!\n");
				break;
			}
		}
		else
		{
			dIndex++;
			if(dIndex==16)
			{
				dIndex=0;
				if(dFlag==0)
					dFlag=1;
				else if(dFlag==1)
					dFlag=0;
			}
			reg=Read_Mem_Dword(eventDequeue+0x10*dIndex+0x0C);
			printf("reg is %llx,dIndex is %llx,dFlag is %llx\n",reg,dIndex,dFlag);
		}
		cnt++;
		if(cnt==16)
			break;
	}
	if(cnt==16)
	{
		printf("Descriptor error by cnt\n");
	}
	if(result == 0)
	{
		printf("Descriptor error\n");
	}
	dIndex++;
	if(dIndex==16)
	{
		dIndex=0;
		if(dFlag==0)
			dFlag=1;
		else if(dFlag==1)
			dFlag=0;
	}
	Write_Mem_Dword(RuntimeBase + 0x18,eventDequeue+0x10*dIndex);		//写回新的Dequeue
	*pIndex=dIndex;
	*pFlag=dFlag;
	return result;
}
UINT32 transferRingConfig(UINT32 TransferRing)
{
	UINT32 reg=0;
	//Setup Stage TRB
	UINT32 wValue=0x0100,bRequest=0x6,bmRequestType=0x80;
	UINT32 wLength=0x12,wIndex=0;
	UINT32 TRBTransferLength=8;
	UINT32 TRT=0,TRBType=0x02,IDT=1,C=0x01;
	reg=(wValue<<16) | (bRequest<<8) | (bmRequestType);
	Write_Mem_Dword(TransferRing+0x00,reg);
	reg=(wLength<<16) | (wIndex);
	Write_Mem_Dword(TransferRing+0x04,reg);
	reg=TRBTransferLength;
	Write_Mem_Dword(TransferRing+0x08,reg);
	reg=(TRT<<16) | (TRBType<<10) | (IDT<<6) | (C);
	Write_Mem_Dword(TransferRing+0x0C,reg);
	//Data Stage TRB
	UINT32 DataBuffer=dmalloc32(0x12, 64);
	memset32(DataBuffer,0,0x12);
	TRBTransferLength=0x12;
	UINT32 DIR=1;
	TRBType=0x03;
	reg=DataBuffer;
	Write_Mem_Dword(TransferRing+0x10+0x00,reg);
	reg=0;
	Write_Mem_Dword(TransferRing+0x10+0x04,reg);
	reg=TRBTransferLength;
	Write_Mem_Dword(TransferRing+0x10+0x08,reg);
	reg=(DIR<<16) | (TRBType<<10) |(C);
	Write_Mem_Dword(TransferRing+0x10+0x0C,reg);
	//Status Stage TRB
	UINT32 IOC=1;
	DIR=0;
	TRBType=4;
	C=1;
	reg=0;
	Write_Mem_Dword(TransferRing+0x10*2+0x00,reg);
	reg=0;
	Write_Mem_Dword(TransferRing+0x10*2+0x04,reg);
	reg=0;
	Write_Mem_Dword(TransferRing+0x10*2+0x08,reg);
	reg=(DIR<<16) | (TRBType<<10) | (IOC<<5) | (C);
	Write_Mem_Dword(TransferRing+0x10*2+0x0C,reg);
	return DataBuffer;
}
UINT32 transferRingInit()
{
	UINT32 TransferRing;
	TransferRing=dmalloc32(0x10*0x10, 64);
	memset32(TransferRing,0,0x10*0x10);
	return TransferRing;
}
void AddDeviceCommand(UINT32 CommandRing,UINT32 eIndex,UINT32 InputContext,UINT32 slotNum)
{
	UINT32 Enqueue=CommandRing + eIndex*0x10;
	UINT32 reg=0,TRBType=11;
	Write_Mem_Dword( Enqueue + 0x0, InputContext);
	Write_Mem_Dword( Enqueue + 0x4, 0);
	Write_Mem_Dword( Enqueue + 0x8, 0);
	reg = (slotNum<<24) | (TRBType<<10) | 0x01;
	Write_Mem_Dword( Enqueue + 0xC, reg);
}
void iEPConfig(UINT32 InputContext,UINT32 Speed,UINT32 tDequeue)
{
	UINT32 iEPContext=InputContext+0x40;
	UINT32 reg=0,MaxPacketSize=0,EPType,AverageTrbLen=0,DCS,TRDequeue;
	if(Speed == 4)
		MaxPacketSize=512;
	else if(Speed == 3)
		MaxPacketSize=64;
	else
		MaxPacketSize=0;
	EPType=4;		//Control
	AverageTrbLen=8;
	DCS=1;			//need to set
	TRDequeue=tDequeue;
	reg=0;
	Write_Mem_Dword(iEPContext,reg);
	reg=(MaxPacketSize<<16) | (EPType<<3);
	Write_Mem_Dword(iEPContext+0x04,reg);
	reg=TRDequeue | (DCS);
	Write_Mem_Dword(iEPContext+0x08,reg);
	Write_Mem_Dword(iEPContext+0x0C,0);
	reg=AverageTrbLen;
	Write_Mem_Dword(iEPContext+0x10,reg);
}
void iSlotConfig(UINT32 InputContext,UINT32 Speed,UINT32 Maxports,UINT32 connectPort)
{
	UINT32 reg=0;
	UINT32 ContextEntry=1,Hub=0,MTT=0,RouteString=0;
	UINT32 MaxExitLatency=0;
	UINT32 IntTarget=0;
	UINT32 iSlotContext=InputContext+0x20;
	connectPort++;
	reg = (ContextEntry<<27) | ( Hub<<26 ) | ( MTT<<25 ) | (Speed<<20) | (RouteString);
	printf("slotContent DWORD(0):%08llx\n",reg);
	Write_Mem_Dword(iSlotContext,reg);
	reg = (Maxports<<24) | (connectPort<<16) | (MaxExitLatency);
	printf("slotContent DWORD(1):%08llx\n",reg);
	Write_Mem_Dword(iSlotContext+0x04,reg);
	reg = (IntTarget<<22);
	printf("slotContent DWORD(2):%08llx\n",reg);
	Write_Mem_Dword(iSlotContext+0x08,reg);
	reg = 0;
	printf("slotContent DWORD(3):%08llx\n",reg);
	Write_Mem_Dword(iSlotContext+0x0C,reg);
}
UINT32 InputContextConfig(UINT32 CapBase)
{
	UINT32 inputContext,dcbaa,reg;
	/*
	UINT32 slotContent,EPContext;	
	slotContent=dmalloc32(0x20, 64);
	memset32(slotContent,0,0x20);
	EPContext=dmalloc32(0x20, 64);
	memset32(EPContext,0,0x20);*/

	reg=Read_Mem_Dword(CapBase+0x10);
	if(reg & DWORD(2))
	{
		inputContext=dmalloc32(0x21*0x40, 64);
		memset32(inputContext,0,0x21*0x40);
	}
	else
	{
		inputContext=dmalloc32(0x21*0x20, 64);
		memset32(inputContext,0,0x21*0x20);
	}
	Write_Mem_Byte(inputContext+0x04,0x03);		//set A1 A0 =  1   1

	return inputContext;
}
UINT8 checkAddress(UINT32 RuntimeBase,UINT32 eventDequeue,UINT32 *pIndex,UINT32 *pFlag,UINT32 slotNum)
{
	UINT32 reg=0,dIndex=*pIndex,dFlag=*pFlag,cnt=0;
	UINT8 result=0;
	reg=Read_Mem_Dword(eventDequeue+0x10*dIndex+0x0C);
	printf("reg is %llx,dIndex is %llx,dFlag is %llx\n",reg,dIndex,dFlag);
	while((reg & 0x01)!=dFlag)
	{
		reg=Read_Mem_Dword(eventDequeue+0x10*dIndex+0x0C);
	}
	printf("2.find some event TRB\n");
	while((reg & 0x01)==dFlag)
	{
		if(((reg & 0x0FC00) == 0x08400) && (Read_Mem_Byte(eventDequeue+0x10*dIndex+0x0B) == 0x01))
		{
			if(Read_Mem_Byte(eventDequeue+0x10*dIndex+0x0C + 0x03)==slotNum)
			{
				result=1;
				printf("Address success!\n");
				break;
			}
		}
		else
		{
			dIndex++;
			if(dIndex==16)
			{
				dIndex=0;
				if(dFlag==0)
					dFlag=1;
				else if(dFlag==1)
					dFlag=0;
			}
			reg=Read_Mem_Dword(eventDequeue+0x10*dIndex+0x0C);
			printf("reg is %llx,dIndex is %llx,dFlag is %llx\n",reg,dIndex,dFlag);
		}
		cnt++;
		if(cnt==16)
			break;
	}
	if(cnt==16)
	{
		printf("address error by cnt\n");
	}
	if(result == 0)
	{
		printf("address error\n");
	}
	dIndex++;
	if(dIndex==16)
	{
		dIndex=0;
		if(dFlag==0)
			dFlag=1;
		else if(dFlag==1)
			dFlag=0;
	}
	Write_Mem_Dword(RuntimeBase + 0x18,eventDequeue+0x10*dIndex);		//写回新的Dequeue
	*pIndex=dIndex;
	*pFlag=dFlag;
	return result;
}
UINT8 readSlot(UINT32 RuntimeBase,UINT32 eventDequeue,UINT32 *pIndex,UINT32 *pFlag)
{
	UINT32 reg=0,dIndex=*pIndex,dFlag=*pFlag,cnt=0;
	UINT8 slotNum=0;
	reg=Read_Mem_Dword(eventDequeue+0x10*dIndex+0x0C);
	printf("reg is %llx,dIndex is %llx,dFlag is %llx\n",reg,dIndex,dFlag);
	while((reg & 0x01)!=dFlag)
	{
		reg=Read_Mem_Dword(eventDequeue+0x10*dIndex+0x0C);
	}
	printf("find some event TRB\n");
	while((reg & 0x01)==dFlag)
	{
		if(((reg & 0x0FC00) == 0x08400) && (Read_Mem_Byte(eventDequeue+0x10*dIndex+0x0B) == 0x01))
		{
			printf("find slot reg is %llx\n",reg);
			slotNum = Read_Mem_Byte(eventDequeue+0x10*dIndex+0x0C + 0x03);
			
			break;
		}
		else
		{
			dIndex++;
			if(dIndex==16)
			{
				dIndex=0;
				if(dFlag==0)
					dFlag=1;
				else if(dFlag==1)
					dFlag=0;
			}
			reg=Read_Mem_Dword(eventDequeue+0x10*dIndex+0x0C);
			printf("reg is %llx,dIndex is %llx,dFlag is %llx\n",reg,dIndex,dFlag);
		}
		cnt++;
		if(cnt==16)
			break;
	}
	if(cnt==16)
	{
		printf("find slot error by cnt\n");
	}
	if(slotNum == 0)
	{
		printf("find slot error\n");
	}
	dIndex++;
	if(dIndex==16)
	{
		dIndex=0;
		if(dFlag==0)
			dFlag=1;
		else if(dFlag==1)
			dFlag=0;
	}
	Write_Mem_Dword(RuntimeBase + 0x18,eventDequeue+0x10*dIndex);		//写回新的Dequeue
	*pIndex=dIndex;
	*pFlag=dFlag;
	return slotNum;
}
UINT8 DisplaySpeed(UINT32 OpBase,UINT32 MaxSlots,UINT32 *pSpeed)
{
	UINT32 reg=0,port=0;
	UINT8 CP=0x0ff;
	UINT32 connectPort=0xFF;
	printf("In disOpBase is %llx \n",OpBase);
	for(UINT32 i=0; i<=MaxSlots; i++)
	{
		reg = Read_Mem_Dword(OpBase + 0x400 + 0x10*i);
		if(reg & DWORD(9))
		{
			printf("port %d : ",port);
			printf(" %08llx ",reg);
			if(reg & 0x01)
			{
				connectPort=port;
				printf("USB device connected. ");
				if(reg & 0x02)
				{
					reg=((reg >> 10) & 0x0F);
					printf("reg is %llx\n",reg);
					*pSpeed=reg;
					switch(reg)     
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
					reg = reg | DWORD(4);
					Write_Mem_Dword(OpBase + 0x400 + 0x10*i,reg);
					reg=Read_Mem_Dword(OpBase + 0x400 + 0x10*i);
					while(!(reg&0x02))
					{
						reg=Read_Mem_Dword(OpBase + 0x400 + 0x10*i);
					}
					//printf("reg is %llx ",reg);
					reg=((reg >> 10) & 0x0F);
					*pSpeed=reg;
					switch(reg)
					{
						case 1:printf("speed full\n");break;
						case 2:printf("speed low\n");break;
						case 3:printf("speed high\n");break;
						case 4:printf("speed super\n");break;
						case 5:printf("speed super plus\n");break;
						default:printf("reserve speed\n");break;
					}
					
				}
			}
			else
			{
				printf("No USB device connected.\n");
			}
			port++;
		}		
	}	
	printf("connectPort is %llx\n",connectPort);
	//printf("*pSpeed is %llx\n",*pSpeed);
	return connectPort;
}
UINT32 EventRingConfig(UINT32 CapBase,UINT32 RuntimeBase)
{
	UINT32 EventRing,HCRunReg,HCRunRegOff,IRS0;
	UINT32 ERST_size=0;
	UINT32 ERST_Entry=0,EventTRB=0;

	IRS0 = RuntimeBase + 0x20;
	/*
	dmalloc32(0x20, 64);
	memset32(IRS0 , 0 , 0x20);
	Write_Mem_Dword(, IRS0);
	*/
	
	EventTRB=dmalloc32(0x10*0x10, 64);
	memset32(EventTRB , 0 , 0x10*10);
	
	ERST_Entry=dmalloc32(0x10, 64);
	memset32(ERST_Entry , 0 , 0x10);
	
	Write_Mem_Word(ERST_Entry + 0x08,0x10);
	Write_Mem_Dword(ERST_Entry + 0x0,EventTRB);
	Write_Mem_Dword(ERST_Entry + 0x4,0);
		
	Write_Mem_Word(IRS0+0x08,1);
	Write_Mem_Dword(IRS0+0x10,ERST_Entry);
	Write_Mem_Dword(IRS0+0x14,0);
	
	Write_Mem_Dword(IRS0+0x18,EventTRB);
	Write_Mem_Dword(IRS0+0x1C,0);
	
	return IRS0;
}
UINT32 CommandRingConfig(UINT32 OpBase)
{
	UINT32 CommandRing;
	CommandRing=dmalloc32(0x10*0x10, 64);
	memset32(CommandRing,0,0x10*0x10);
	//CommandRing = CommandRing ;  
	if(Read_Mem_Dword(OpBase+0x18) & DWORD(3))
	{
		printf("command ring is runing,can't write the command ring address");
	}
	
	Write_Mem_Dword(OpBase+0x18,CommandRing | DWORD(0));
	Write_Mem_Dword(OpBase+0x18+0x4,0x0);
	printf("Read 0x18 cmd is %llx\n",Read_Mem_Dword(OpBase+0x18));
	Write_Mem_Dword(CommandRing+0x10*(0x10-1)+0xC,0x01800);
	Write_Mem_Dword(CommandRing+0x10*(0x10-1),CommandRing);

	
	return CommandRing;
}
UINT32 DCBAAConfig(UINT32 CapBase,UINT32 OpBase,UINT32 MaxSlots)
{
	UINT32 DeviceContext,dcbaa,reg;
	reg=Read_Mem_Dword(CapBase+0x10);
	if(reg & DWORD(2))
	{
		DeviceContext=dmalloc32(0x20*0x20, 64);
		memset32(DeviceContext,0,0x20*0x20);
	}
	else
	{
		DeviceContext=dmalloc32(0x20*0x40, 64);
		memset32(DeviceContext,0,0x20*0x40);
	}
	dcbaa=dmalloc32((MaxSlots+1)*8, 64);
	memset32(dcbaa,0,(MaxSlots+1)*8);

	Write_Mem_Dword(dcbaa+0x8,DeviceContext);			//slot1
	Write_Mem_Dword(dcbaa+0xc,0);

	Write_Mem_Dword(OpBase+0x30,dcbaa);
	Write_Mem_Dword(OpBase+0x34,0x0);

	return dcbaa;
}
UINT8 GetMaxPortsByCap(UINT32 CapBase)
{
	UINT8 reg=Read_Mem_Byte(CapBase+0x04+0x03);
	reg=reg;
	return reg;
}
UINT8 GetMaxSlotsByCap(UINT32 CapBase)
{
	UINT32 reg=Read_Mem_Dword(CapBase+0x04);
	reg=reg & 0x0ff;
	return (UINT8)reg;
}
void SetMaxSlots(UINT32 OpBase,UINT8 MaxSlots)
{
	Write_Mem_Byte(OpBase+0x38,MaxSlots);
}
void XHCI_controller_Run(UINT32 OpBase)
{
	UINT32 cnt=100;
	UINT32 reg=Read_Mem_Dword(OpBase);
	reg=reg | DWORD(0);
	Write_Mem_Dword(OpBase,reg);
	reg=Read_Mem_Dword(OpBase+0x04);
	delay(16);
	/*
	printf("----------------------------Run-------------------------------------\n");
	while(cnt--)
	{
		reg=Read_Mem_Dword(OpBase+0x04);
		printf("%08llx  %08llx\n",reg,reg&DWORD(0));
	}
	printf("----------------------------Run-------------------------------------\n");
	*/
	while(reg&DWORD(0))
	{
		reg=Read_Mem_Dword(OpBase+0x04);
	}
	//printf("----------------------------Run-------------------------------------\n");
	//printf("%08llx\n",Read_Mem_Dword(OpBase+0x04));
	//printf("----------------------------Run-------------------------------------\n");	
		
}
void XHCI_controller_Stop(UINT32 OpBase)
{
	UINT32 cnt=100;
	UINT32 reg=Read_Mem_Dword(OpBase);
	reg=reg & ~DWORD(0);
	Write_Mem_Dword(OpBase,reg);
	delay(16);
	/*
	printf("----------------------------Stop-------------------------------------\n");
	while(cnt--)
	{
		reg=Read_Mem_Dword(OpBase+0x04);
		printf("%08llx  %08llx\n",reg,reg&DWORD(0));
	}
	printf("----------------------------Stop-------------------------------------\n");
	*/

	reg=Read_Mem_Dword(OpBase+0x04);
	while((reg & DWORD(0))!=0x00000001UL)
		reg=Read_Mem_Dword(OpBase+0x04);
	//printf("----------------------------Stop-------------------------------------\n");
	//printf("%08llx\n",Read_Mem_Dword(OpBase+0x04));
	//printf("----------------------------Stop-------------------------------------\n");
}
void XHCI_controller_Reset(UINT32 OpBase)
{
	UINT32 reg=Read_Mem_Dword(OpBase);
	reg=reg | DWORD(1);
	Write_Mem_Dword(OpBase,reg);
	reg=Read_Mem_Dword(OpBase);
	while(reg & DWORD(1))
		reg=Read_Mem_Dword(OpBase);
}
void ReadOpBase(UINT32 *CapBaseArray,UINT32 *OpBaseArray,UINT32 Cnt)
{
    UINT32 i,CapBase,OpBase,reg;
    for(i=0;i<Cnt;i++)
	{
		CapBase=CapBaseArray[i];
		OpBaseArray[i]=CapBase+Read_Mem_Byte(CapBase);
		OpBase=OpBaseArray[i];
		//printf("CapBase is %llx,OpBase is %llx\n",CapBase,OpBase);
	}
}
void ReadCAPBase(UINT32 *CapBase,UINT32 *Cnt)
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
					if((result & 0xFFFFFF00) == 0x0C033000)
					{
						cmd_temp = cmd + 0x10;
						outport_long(PCI_CONFIG_ADDR_PORT,cmd_temp);
						result=inport_long(PCI_CONFIG_DATA_PORT);
						*(CapBase+*Cnt)=result & 0xFFFFFFF0;
						(*Cnt)++;
					}
				}
			}
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
