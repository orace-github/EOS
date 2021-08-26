global __start
extern main

MAGIC_NUMBER	equ	0x1BADB002
FLAGS			equ	0x03
CHECKSUM		equ	-(MAGIC_NUMBER + FLAGS)


__start:
	jmp loader

align 4
multiboot_header:
dd MAGIC_NUMBER
dd FLAGS
dd CHECKSUM

loader:
	push ebx
	call main

	cli
	hlt
	