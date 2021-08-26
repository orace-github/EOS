%macro EOI_MASTER 0
mov al,0x20
out 0x20,al
%endmacro

%macro EOI_SLAVE 0
mov al,0x20
out 0xa0,al
%endmacro

global syscall
extern __isrsyscall
syscall:
	pushad
	push ds
	push es
	push fs
	push gs
	push byte 0x80; interrupt number
	mov ebx,ds; save original data segment
	push ebx
	mov bx,0x10; kernel data segment
	mov ds,bx
	mov es,bx
	mov fs,bx
	mov gs,bx
	pop ebx
	push esp; registers_t @
	call __isrsyscall
	pop esp
	pop eax; pop interrupt number from stack
	pop gs
	pop fs
	pop es
	pop ds
	popad
	EOI_MASTER
	EOI_SLAVE
	iret