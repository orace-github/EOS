%macro EOI_MASTER 0
mov al,0x20
out 0x20,al
%endmacro

%macro EOI_SLAVE 0
mov al,0x20
out 0xa0,al
%endmacro

extern final_exc_handler
%macro EXC 1
	global exc%1
exc%1:
	cli
	pushad
	push ds
	push es
	push fs
	push gs
	push byte %1; exception number
	mov ebx,ds; save original data segment
	push ebx
	mov bx,0x10; kernel data segment
	mov ds,bx
	mov es,bx
	mov fs,bx
	mov gs,bx
	pop ebx
	push esp; registers_t @
	call final_exc_handler
	pop esp
	pop eax; pop interrupt number from stack
	pop gs
	pop fs
	pop es
	pop ds
	popad
	EOI_MASTER
	EOI_SLAVE
	sti
	iret
%endmacro

;; ISR wrap ;;
EXC 0
EXC 1
EXC 2
EXC 3
EXC 4
EXC 5
EXC 6
EXC 7
EXC 8
EXC 9
EXC 10
EXC 11
EXC 12
EXC 13
EXC 14
EXC 15
EXC 16
EXC 17
EXC 18
EXC 19
EXC 20
EXC 21
EXC 22
EXC 23
EXC 24
EXC 25
EXC 26
EXC 27
EXC 28
EXC 29
EXC 30
EXC 31