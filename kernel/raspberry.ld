ENTRY(_start)

SECTIONS
{
    . = 0x8000;

    .text : {
        *(.text*)
        *(.rodata*)
    }
    
    .init_array : {
        __init_array_start = ABSOLUTE(.);;
        *(.init_array)
        *(SORT_BY_INIT_PRIORITY(.init_array.*))
        __init_array_end = ABSOLUTE(.);;
    }

    .data : {
        *(.data*) 
    }

    .bss : {
        __bss_start__ = ABSOLUTE(.);
        *(.bss*)
        *(COMMON)
        __bss_end__ = ABSOLUTE(.);
    }
    
    .ARM.exidx : {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } 
    
    .stack : {
        __stack_start__ = ABSOLUTE(.);

        . += 262144;
        __irq_stack_top__ = ABSOLUTE(.);

        . += 262144;
        __c_stack_top__ = ABSOLUTE(.);

        __stack_end__ = ABSOLUTE(.);
    }

    __heap_start = ABSOLUTE(.);
    __heap_end = 0x1C000000;
}
