#include<stdio.h>
#include<conio.h>
#include <dos.h>
#define  DataPort 0x60
unsigned char flag=1;
void clareInt09()
{
	printf("Enter the interrupt module\n");
	disable();
	unsigned char scanCode;
	scanCode=inportb(DataPort);
	printf("scanCode is %x\n",scanCode);
	outportb(0x80, scanCode);
	if(scanCode==0x01)
		flag=0;
	enable();
	outportb(0x20,0x20);
}
int main()
{
	unsigned int SegmentIntSer,OffsetIntSer;
	unsigned int segmentaddr,offsetaddr;
	unsigned char IntVecIndex=36;
	OffsetIntSer=FP_OFF(clareInt09);
	SegmentIntSer=FP_SEG(clareInt09);

	printf("new segment is %x ",SegmentIntSer);
	printf("new offset is %x \n",OffsetIntSer);

	printf("Interrupt Vector Index is 0x%x\n",9*4);
	asm{
		XOR AX, AX
		MOV ES, AX
		MOV BX, IntVecIndex
		mov bh, 00h
		MOV AX, ES:word ptr[BX]
		MOV offsetaddr,AX
		MOV AX, ES:word ptr[BX+2]
		MOV segmentaddr,AX
	}
	printf("old segment is %x ",segmentaddr);
	printf("old offset is %x \n",offsetaddr);
	asm{
		XOR AX, AX
		MOV ES, AX
		MOV BX, IntVecIndex
		mov bh, 00h
		MOV AX,OffsetIntSer
		MOV ES:word ptr[BX], AX
		MOV AX, SegmentIntSer
		MOV ES:word ptr[BX+2], AX
	}

	while(flag)
	{

	}
	asm{
		XOR AX, AX
		MOV ES, AX
		MOV BX, IntVecIndex
		mov bh, 00h
		MOV AX, offsetaddr
		MOV ES:word ptr[BX], AX
		MOV AX, segmentaddr
		MOV ES:word ptr[BX+2], AX
		MOV  AH, 0x4C
		INT  0X21
	}

//	getchar();

	return 0;
}