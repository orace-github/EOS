ENTRY(__start)                /* le nom de l'étiquette du point d'entrée */


SECTIONS {

    . = 1M;          /* le code doit être chargé à 1 Mo */
	__kernel_base = .;
	
    .text BLOCK(4K) : ALIGN (4K)   /* alignement à 4 ko */
    {	
		*(.multiboot)
        *(.text)             /* toutes les sections texte de tous les fichiers */
    }
	
	
    .rodata BLOCK(4K) : ALIGN (4K) /* alignement à 4 ko */
    {
        *(.rodata*)          /* toutes les sections de données en lecture seule, de tous les fichiers */
    }
	
	
    .data BLOCK(4K) : ALIGN (4K)   /* alignement à 4 ko */
    {
        *(.data)             /* toutes les sections de données, de tous les fichiers */
    }


    .bss BLOCK(4K) : ALIGN (4K)    /* alignement à 4 ko */
    {
        *(COMMON)            /* toutes les sections COMMON de tous les fichiers */
        *(.bss)              /* toutes les sections bss de tous les fichiers */
    }
    
    __kernel_end = ALIGN(4K);
    
}
