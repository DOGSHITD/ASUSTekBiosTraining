#include<stdio.h>
#include<dos.h>
void main()
{
	typedef unsigned int UINT;
	unsigned char hAddr,lAddr,hValue,lValue,BankSelect,count=3;
    unsigned int HWMbase,AddrPort,FanSpeed,DataPort;
	//Enter MB PnP Mode(Unlock)
	outportb(0x2e,0x87);
	outportb(0x2e,0x87);
	
	//LDN select HARDWARE MONITOR, FRONT PANEL LED
	outportb(0x2e,0x07);
	outportb(0x2f,0x0B);

	//LDN(Logical Device) Active
	outportb(0x2e,0x30);
	outportb(0x2f,0x01);
	
	//Read HWMbase
	outportb(0x2e,0x60);
	hAddr = inportb(0x2f);
	printf("0x60 is 0x%x\n",hAddr);

	outportb(0x2e,0x61);
	lAddr = inportb(0x2f);
	//printf("0x61 is 0x%x\n",lAddr);

	HWMbase = ((UINT)hAddr<<8) | lAddr;
	//printf("HWMbase is 0x%x\n",HWMbase);

	AddrPort = HWMbase+0x05;
	DataPort = HWMbase+0x06;
	//printf("AddrPort is 0x%x\n",AddrPort);
	//printf("DataPort is 0x%x\n",DataPort);

	outportb(AddrPort,0x4e);
	outportb(DataPort,0x04);
	while(count--)
	{
		outportb(AddrPort,0xc2);
		hValue=inportb(DataPort);
		printf("hValue is 0x%x\n",hValue);

		outportb(AddrPort,0xc3);
		lValue=inportb(DataPort);
		printf("lValue is 0x%x\n",lValue);
		
		FanSpeed = ((UINT)hValue<<8) | lValue;
		printf("FanSpeed is %d\n",FanSpeed);
		delay(3000);
	}

	outportb(0x2e,0xaa);//Exit MB PnP Mode(Lock)
}