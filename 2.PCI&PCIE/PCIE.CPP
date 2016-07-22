#include<stdio.h>
#include<dos.h> 
#define BaseAddr 0xF8000000
typedef unsigned long DWORD;

//open protect mode
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

DWORD ReadMemByPM(DWORD addr)
{
    DWORD result;
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
/*
void writelog(DWORD mAddr[],DWORD count,DWORD len)
{
	FILE *fp;
	fp=fopen("PCIE_list.log","w+");
	if(fp==NULL)return;
	DWORD cia;
	fprintf(fp,"begin to write\n",mAddr[cia]);
	for(cia=0;cia<count;cia++)
	{
		fprintf(fp," addr is %8.8llx\n",mAddr[cia]);
		DWORD addr=0;
		DWORD result=0;
		DWORD row = len/16;
		DWORD i=15,j=0;
		fprintf(fp," offset ");
		for(;1;i--)
		{
			fprintf(fp," %X",i);
			if(i%4==0)
				fprintf(fp," ");
			if(i==0)
			     break;
		}
		i=0;
		fprintf(fp,"\n-------------------------------------------------\n");
		for(i=0;i<row;i++)
		{
			fprintf(fp," %dF..%d0 ",i,i);
			for(j=12;1;j=j-4)
			{
				addr=mAddr[cia]+i*16+j;
				result=ReadMemByPM(addr);
				fprintf(fp,"%8.8llx ",result);
				if(j==0)
				 break;
			}
			fprintf(fp,"\n");
		}
	}
	fclose(fp);
}*/
void listAllPCI()
{
	FILE *fp;
	fp=fopen("PCIE.log","w+");
	if(fp==NULL)return;
	DWORD busNum,devNum=0,funNum=0,result=0,addr,CabID,mOffset,DevPortType;
	DWORD vendorID,deviceID,LinkWidth,LinkSpeed;
	DWORD flag=0,count=0;
	DWORD mAddr[20];
	char TypeIndex=9,PortType[10][40]={"PCI Express Endpoint","Legacy PCI Express Endpoint","Root Port of PCI Express Root Complex","Upstream Port of PCI Express Switch","Downstream Port of PCI Express Switch","PCI Express to PCI/PCI-X Bridge","PCI/PCI-X to PCI Express Bridge","Root Complex Integrated Endpoint","Root Complex Event Collector","None"};
	fprintf(fp,"\n-------------------   Lets'go!      ---------------------\n");
	fprintf(fp,"ID address  vID    dID   Width  Speed        Type\n");
	printf("ID address  vID    dID   Width  Speed        Type\n");
	for(busNum=0;busNum<64;busNum++)
		for(devNum=0x0;devNum<32;devNum++)
			for(funNum=0;funNum<8;funNum++)
			{

				addr=(0xF8000000)|(busNum<<20)|(devNum<<15)|(funNum<<12);
				fprintf(fp,"addr is hahaha\n");
				result=ReadMemByPM(addr)>>16;
				if(result!=0x0000ffff)
				{
					result=ReadMemByPM(addr|0x34);
					mOffset=result&0x000000ff;
					flag=0;
					LinkWidth=0;
					LinkSpeed=0;
					TypeIndex=9;
					while(1)
					{
						result=ReadMemByPM(addr|mOffset);
						CabID=result&0x000000ff;
						if(CabID != 0x00000010)
						{
							mOffset=(result&0x0000ff00)>>8;
							if(mOffset==0x00000000)
								break;
						}
						else
						{
							mAddr[count++]=addr;// store addr in maddr to write log
							result=ReadMemByPM(addr);
							vendorID=result&0x0000ffff;
							deviceID=(result&0xffff0000)>>16;
							result=ReadMemByPM(addr|mOffset);
							DevPortType=(result&0x00f00000)>>20;
							switch(DevPortType)
								{
									case 0x0:TypeIndex=0;break;
									case 0x1:TypeIndex=1;break;
									case 0x4:TypeIndex=2;break;
									case 0x5:TypeIndex=3;break;
									case 0x6:TypeIndex=4;break;
									case 0x7:TypeIndex=5;break;
									case 0x8:TypeIndex=6;break;
									case 0x9:TypeIndex=7;break;
									case 0xa:TypeIndex=8;break;
									default:break;
								}
							result=ReadMemByPM(addr+mOffset+0x0c);
							LinkWidth=(result&0x000003f0)>>4;
							result=ReadMemByPM(addr+mOffset+0x0c);
							LinkSpeed=result&0x0000000f;
							flag=1;
							break;
						}
					}
					if(flag==1)
					{
						//fprintf(fp,"%llx\n",count);
						printf("%llx.%8.8llx  %4.4llx   %4.4llx   X%2lu   %2lu.0     %s\n",count,addr,vendorID,deviceID,LinkWidth,LinkSpeed,PortType[TypeIndex]);
						//fprintf(fp,"%llx.%8.8llx  %4.4llx   %4.4llx   X%2lu   %2lu.0     %s\n",count,addr,vendorID,deviceID,LinkWidth,LinkSpeed,PortType[TypeIndex]);
						fprintf(fp,"1     \n");
					}
				}
			}
	fclose(fp);
//	writelog(mAddr,count,0x40);
}
int main()
{
	openA20();
	set4gb();
	printf("i am coming\n");
	DWORD result=0;
	result=ReadMemByPM(0xF8000000);
	printf("result is %8.8llx\n",result);
	listAllPCI();
	return 0;
}
