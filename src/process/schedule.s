%macro EOI_MASTER 0
mov al,0x20
out 0x20,al
%endmacro

%macro EOI_SLAVE 0
mov al,0x20
out 0xa0,al
%endmacro

global do_switch
do_switch:
cli
mov esi, dword [esp] ;get @registers 
pop eax ; pop @registers
push dword [esi + 4]; gs
push dword [esi + 8]; fs
push dword [esi + 12];es
push dword [esi + 16];ds
push dword [esi + 24];edi
push dword [esi + 28];esi
push dword [esi + 32];ebp
push dword [esi + 40];ebx
push dword [esi + 44];edx
push dword [esi + 48];ecx
push dword [esi + 52];eax
pop eax
pop ecx
pop edx
pop ebx
pop ebp
pop esi
pop edi
pop ds
pop es
pop fs
pop gs
EOI_MASTER
EOI_SLAVE
iret
