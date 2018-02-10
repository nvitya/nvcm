
/* Mandatory to be word aligned, _sbrk assumes this */
PROVIDE ( end = _end_noinit ); /* was _ebss */
PROVIDE ( _end = _end_noinit );
PROVIDE ( __end = _end_noinit );
PROVIDE ( __end__ = _end_noinit );

._check_stack : ALIGN(4)
{
    . = . + _Minimum_Stack_Size ;
} > RAM
