#include <stdio.h> 
#include <dos.h>

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



void main()
{
	unsigned long result=0;
	unsigned long addr;
	openA20();
	set4gb();
	asm push ds
	asm mov ax,0
	asm mov ds,ax
	asm mov esi,0F8000000h
	N1: asm mov eax,[esi]
	asm mov result,eax;
	asm mov dword ptr [esi],0
	asm mov ebx,[esi]
	asm mov dword ptr [esi],0ffffffffh
	asm mov ecx,[esi]
	asm mov [esi],eax
	asm inc ecx
	asm cmp ebx,ecx
	asm jne N2
	asm add esi,4
	asm jmp N1
	N2: asm mov addr,esi
	asm pop ds
	printf("RAM = %ld(Mb)\n", addr >> 20 );
	printf("result = %llx\n",result);

}
