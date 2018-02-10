/* must be included into a section */

KEEP(*(.init))
KEEP(*(.fini))

. = ALIGN(4);

PROVIDE_HIDDEN (__preinit_array_start = .);
KEEP(*(.preinit_array_sysinit .preinit_array_sysinit.*))
KEEP(*(.preinit_array_platform .preinit_array_platform.*))
KEEP(*(.preinit_array .preinit_array.*))
PROVIDE_HIDDEN (__preinit_array_end = .);

. = ALIGN(4);

PROVIDE_HIDDEN (__init_array_start = .);
KEEP(*(SORT(.init_array.*)))
KEEP(*(.init_array))
PROVIDE_HIDDEN (__init_array_end = .);

. = ALIGN(4);

PROVIDE_HIDDEN (__fini_array_start = .);
KEEP(*(SORT(.fini_array.*)))
KEEP(*(.fini_array))
PROVIDE_HIDDEN (__fini_array_end = .);
