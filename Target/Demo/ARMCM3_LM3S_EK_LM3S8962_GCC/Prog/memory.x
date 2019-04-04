MEMORY
{
    FLASH (rx) : ORIGIN = 0x00004000, LENGTH = 240K
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}

SECTIONS
{
    __STACKSIZE__ = 1024;

    .text :
    {
		    KEEP(*(.isr_vector))
		    *(.text*)

		    KEEP(*(.init))
		    KEEP(*(.fini))

		    /* .ctors */
		    *crtbegin.o(.ctors)
		    *crtbegin?.o(.ctors)
		    *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
		    *(SORT(.ctors.*))
		    *(.ctors)

		    /* .dtors */
     		*crtbegin.o(.dtors)
     		*crtbegin?.o(.dtors)
     		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
     		*(SORT(.dtors.*))
     		*(.dtors)

		    *(.rodata*)

		    KEEP(*(.eh_frame*))
        _etext = .;
    } > FLASH

    .data : AT (ADDR(.text) + SIZEOF(.text))
    {
        _data = .;
		    *(vtable)
		    *(.data*)

		    . = ALIGN(4);
		    /* preinit data */
		    PROVIDE_HIDDEN (__preinit_array_start = .);
		    KEEP(*(.preinit_array))
		    PROVIDE_HIDDEN (__preinit_array_end = .);

		    . = ALIGN(4);
		    /* init data */
		    PROVIDE_HIDDEN (__init_array_start = .);
		    KEEP(*(SORT(.init_array.*)))
		    KEEP(*(.init_array))
		    PROVIDE_HIDDEN (__init_array_end = .);


		    . = ALIGN(4);
		    /* finit data */
		    PROVIDE_HIDDEN (__fini_array_start = .);
		    KEEP(*(SORT(.fini_array.*)))
		    KEEP(*(.fini_array))
		    PROVIDE_HIDDEN (__fini_array_end = .);

		    . = ALIGN(4);
        _edata = .;
    } > SRAM

    .bss :
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
        _stack = .;
        . = ALIGN(MAX(_stack + __STACKSIZE__ , .), 4);
        _estack = .;
        
    } > SRAM
}
