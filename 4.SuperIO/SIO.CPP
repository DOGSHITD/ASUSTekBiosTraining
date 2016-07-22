#include<stdio.h>
#include <dos.h>
#include <conio.h>

int main()
{	
	


	//Enter MB PnP Mode(Unlock)
	unsigned char cData,count=3;
	outportb(0x2e,0x87);
	outportb(0x2e,0x87);
	
	//set LDN as GPIO2, GPIO3, GPIO4, GPIO5 chao by jiang
	outportb(0x2e,0x07);
	outportb(0x2f,0x09);
	
	//LDN(Logical Device) Active
	outportb(0x2e,0x30);
	cData=inportb(0x2f);
	cData=cData | 0x01;
	outportb(0x2f,cData);
	
	//Select GPIO function
	outportb(0x2e,0x2c);
	cData=inportb(0x2f);
	cData=cData&0xE7;
	outportb(0x2f,cData);
	
	//Select GPI or GPO for GPIO
	outportb(0x2e,0xe0);
	outportb(0x2f,0x00);
	

	
	while(count--)
	{
		outportb(0x2e,0xe1);
		outportb(0x2f,0x00);
		outportb(0x80,0xaa);
		delay(3000);

		outportb(0x2e,0xe1);
		outportb(0x2f,0xFF);
		outportb(0x80,0x55);
		delay(3000);
	}
	
	outportb(0x2e,0xaa);
	return 0;
}