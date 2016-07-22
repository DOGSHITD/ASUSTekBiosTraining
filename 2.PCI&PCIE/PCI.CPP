#include<stdio.h>
#define PCI_CONFIG_ADDR_PORT 0x0CF8
#define PCI_CONFIG_DATA_PORT 0x0CFC
typedef unsigned long DWORD;

void outport_long(int portNum,DWORD cmd);
DWORD inport_long(int portNum);

void main()
{
	//outport_long(0x0CF8,0x80000000);
	DWORD busNum,devNum,funNum;
	DWORD baseCmd=0x80000000,cmd;
	DWORD result=0;
	DWORD i,j,k;
	FILE *fp;
	fp=fopen("PCI.log","w+");
	if(fp==NULL)return;
	printf("busNum   Device   funNum     result\n");
	fprintf(fp,"busNum   Device   funNum     result\n");
	for(i=0;i<256;i++)//for bus number
		for(j=0;j<32;j++)//for device id
			for(k=0;k<8;k++)
			{
				busNum=i;devNum=j;funNum=k;
				cmd=baseCmd|(busNum<<16)|(devNum<<11)|(funNum<<8);

				outport_long(PCI_CONFIG_ADDR_PORT,cmd);
				result=inport_long(PCI_CONFIG_DATA_PORT);
				if(result!=0xffffffff)
				{
					//printf("0x%llx\n",result);
					printf("%2llx       %2llx       %2llx       0x%llx\n",busNum,devNum,funNum,result);
					fprintf(fp,"%2llx       %2llx       %2llx       0x%llx\n",busNum,devNum,funNum,result);
					//printf("0x%llx\n",result);
					//printf("%x %x %x %llx\n",0x87,0x65,0x43,0x87654321);
				}
			}
	fclose(fp);
	//DWORD result;//=inport_long(0x0CFC);
	//	printf("the result is 0X%8X\n",result);
}

void outport_long(int portNum,DWORD cmd)
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
DWORD inport_long(int portNum)
{
       /*	printf("--------in the inport function----------\n");
	printf("this portNum is 0x%X\n",portNum);
	*/
	DWORD result=0;
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
