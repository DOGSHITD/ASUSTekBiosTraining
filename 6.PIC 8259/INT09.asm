;�Эɭɭɭɭɭɭɭɭɭɭɭɭɭɭɭɭɭɭɭɭɭɭɭɭɭ�
;�� EXE�ļ�������Ľṹ��MASM6.0��  �εļ򻯷�ʽʾ�� ��
;�ϭ˭˭˭˭˭˭˭˭˭˭˭˭˭˭˭˭˭˭˭˭˭˭˭˭˭�

TITLE	EXE6p  (exe)	Clare  2015-08-04	;TITLEΪ����ע�ͷ�

.MODEL  small        		; EXE�ļ�(Сģʽ,CODE��DATA��64KB).
.STACK  256 			; ��ջ��,��ջ�ռ�=256
.DATA                   	; ���ݶ�
	IntMode  DB 'Enter the interrupt module. ScanCode is ','$'	; �ַ����洢��, '$'�ַ���������־
	MSG1  DB 'Success!','$'	; �ַ����洢��, '$'�ַ���������־
	flag  DB 30h,65h,6ch,00h
	oldSeg      DW 0h
	oldOffset   DW 0; 
.CODE                   	; �����
        .STARTUP                ; ��ʼ��DATA��DS=DATA,��SS=DS

;---------------------------------------------
;	��Ĵ���...

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
	mov  dx,offset IntMode  ; dx=�ַ���'Enter the interrupt module'��ƫ�Ƶ�ַ
	mov  ah,09h		; ���ܺ�ah=9 (��ʾ�ַ���)
	int  21h		; 21h=DOSϵͳ�����жϵ���,��ʾ�ַ���'Interrupt mode!'

	in al,60h
	mov flag,al
	out 80h,al
	call disp_al		
        

	mov dl,13		;�س�
	mov ah,2
	int 21h
	mov dl,10
	mov ah,2
	int 21h

	sti		
	mov al,20h
	out 20h,al
	iret
	
			
disp_al:		;disp_al�ӳ��򽫼Ĵ���al�е���ֵ��ʮ��������ʾ����
	mov dh, al
	
	mov bx, 2	;bx����ѭ��,һ���ֽ���������ʾ�������ַ�,����ѭ��������2��
	
	mov cl, 4	;������ʾ��4λ,���´�ѭ��������ʾ��4λ
	shr al, cl
again:
	and al, 0fh
	cmp al, 9
	ja hex
	add al, '0'	;���С��9,�����ַ�'0'������ת���ַ�
	jmp disp
hex:
	sub al, 10	;�������9,�ȼ�ȥ10,�ټ����ַ�'A',�Ϳ��Խ�����ת������Ӧ��'A'~'F'
	add al, 'A'
disp:			;�����ж���ʾ�ַ�
	mov dl, al
	mov ah, 02h
	int 21h
	
	mov al, dh
	
	dec bx
	test bx, bx
	jnz again

	ret


cEnd:
	mov  dx,offset MSG1	; dx=�ַ���'Hello!'��ƫ�Ƶ�ַ
	mov  ah,09h		; ���ܺ�ah=9 (��ʾ�ַ���)
	int  21h		; 21h=DOSϵͳ�����жϵ���,��ʾ�ַ���'Hello!'

	XOR AX, AX
	MOV ES, AX
	MOV BX, 24h
	mov bh, 00h
	MOV AX,oldOffset
	MOV ES:word ptr[BX], AX
	MOV AX,oldSeg
	MOV ES:word ptr[BX+2], AX

	
	MOV	AH, 0		;�������ַ�
	INT	16H		;�����̻�����Ϊ�գ�һֱ�ȴ������ַ�

        .EXIT 0               	; ����DOS,������= 0

	END			; ������