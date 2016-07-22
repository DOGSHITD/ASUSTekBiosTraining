#include <stdio.h>
typedef unsigned long DWORD;
void main()
{
   DWORD addr,result;
   addr=0xF8000000;
    DWORD *p=0;
   p=(DWORD *)addr;
   result=*p;
   printf("*p is %llx\n",result);
   _asm{
	push eax;
	push ds;
	mov ax,0;
	mov ds,ax;
	mov esi,addr;
	mov eax,[esi];
	mov result,eax;
	pop ds;
	pop eax;
   }
   printf("[F8000000]=0x%llx\n",result);

}