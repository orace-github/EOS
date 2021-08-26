%macro EOI_MASTER 0
mov al,0x20
out 0x20,al
%endmacro

%macro EOI_SLAVE 0
mov al,0x20
out 0xa0,al
%endmacro

extern final_irq_handler
%macro IRQ 1
	global irq%1
irq%1: 
	cli
	pushad
	push ds
	push es
	push fs
	push gs
	push byte %1; interrupt number
	mov ebx,ds; save original data segment
	push ebx
	mov bx,0x10; kernel data segment
	mov ds,bx
	mov es,bx
	mov fs,bx
	mov gs,bx
	pop ebx
	push esp; registers_t @
	call final_irq_handler
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
IRQ 0
IRQ 1
IRQ 2
IRQ 3
IRQ 4
IRQ 5
IRQ 6
IRQ 7
IRQ 8
IRQ 9
IRQ 10
IRQ 11
IRQ 12
IRQ 13
IRQ 14
IRQ 15
IRQ 255; default interrupt





