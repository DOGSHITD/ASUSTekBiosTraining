;
; EXE文件汇编程序的结构【MASM6.0】  段的简化方式示例 
;

TITLE	EXE6p  (exe)	Clare  2015-08-04	;TITLE为标题注释符

.MODEL  small        		; EXE文件(小模式,CODE和DATA各64KB).
.STACK  256 			; 堆栈段,堆栈空间=256
.DATA                   	; 数据段
	IntMode  DB 'Enter the interrupt module. ScanCode is ','$'	; 字符串存储区, '$'字符串结束标志
	MSG1  DB 'Success!','$'	; 字符串存储区, '$'字符串结束标志
	flag  DB 30h,65h,6ch,00h
	oldSeg      DW 0h
	oldOffset   DW 0; 
.CODE                   	; 代码段
        .STARTUP                ; 初始化DATA段DS=DATA,且SS=DS

;---------------------------------------------
;	你的代码...

;---------------------------------------------
	XOR AX, AX
	MOV ES, AX
	MOV BX, 24h
	mov bh, 00h
	MOV AX, ES:word ptr[BX]
	MOV oldOffset,AX
	MOV AX, ES:word ptr[BX+2]
	MOV oldSeg,AX

	XOR AX, AX
	MOV ES, AX
	MOV BX, 24h
	mov bh, 00h
	MOV AX,offset clareInt09
	MOV ES:word ptr[BX], AX
	MOV AX,seg clareInt09
	MOV ES:word ptr[BX+2], AX
	deathLoop:
	mov dl,01h
	cmp dl,flag
	jz cEnd
	jmp deathLoop

	clareInt09:
	cli
	mov  dx,offset IntMode  ; dx=字符串'Enter the interrupt module'的偏移地址
	mov  ah,09h		; 功能号ah=9 (显示字符串)
	int  21h		; 21h=DOS系统功能中断调用,显示字符串'Interrupt mode!'

	in al,60h
	mov flag,al
	out 80h,al
	call disp_al		
        

	mov dl,13		;回车
	mov ah,2
	int 21h
	mov dl,10
	mov ah,2
	int 21h

	sti		
	mov al,20h
	out 20h,al
	iret
	
			
disp_al:		;disp_al子程序将寄存器al中的数值以十六进制显示出来
	mov dh, al
	
	mov bx, 2	;bx用于循环,一个字节最多可以显示成两个字符,所以循环次数是2次
	
	mov cl, 4	;首先显示高4位,在下次循环进再显示低4位
	shr al, cl
again:
	and al, 0fh
	cmp al, 9
	ja hex
	add al, '0'	;如果小于9,加上字符'0'将数字转换字符
	jmp disp
hex:
	sub al, 10	;如果大于9,先减去10,再加上字符'A',就可以将数字转换成相应的'A'~'F'
	add al, 'A'
disp:			;利用中断显示字符
	mov dl, al
	mov ah, 02h
	int 21h
	
	mov al, dh
	
	dec bx
	test bx, bx
	jnz again

	ret


cEnd:
	mov  dx,offset MSG1	; dx=字符串'Hello!'的偏移地址
	mov  ah,09h		; 功能号ah=9 (显示字符串)
	int  21h		; 21h=DOS系统功能中断调用,显示字符串'Hello!'

	XOR AX, AX
	MOV ES, AX
	MOV BX, 24h
	mov bh, 00h
	MOV AX,oldOffset
	MOV ES:word ptr[BX], AX
	MOV AX,oldSeg
	MOV ES:word ptr[BX+2], AX

	
	MOV	AH, 0		;读键入字符
	INT	16H		;若键盘缓冲区为空，一直等待键入字符

        .EXIT 0               	; 返回DOS,返回码= 0

	END			; 汇编结束