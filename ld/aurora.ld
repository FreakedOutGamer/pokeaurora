OUTPUT_ARCH(arm)
ENTRY(_start)

INCLUDE syms.ld

/* Sections are laid out in the order the objects are given to the linker,
   then agbcc's libgcc and libc (libgcc, libc, libgcc again). */
SECTIONS
{
    . = 0x08000000;

    .text :
    ALIGN(4)
    {
        *(.text);
        . = ALIGN(4);
    } =0

    .rodata :
    ALIGN(4)
    {
        *(.rodata);
        . = ALIGN(4);
    } =0

    /* Copied to EWRAM by AgbMain. */
    .data 0x02000000 :
    AT(ALIGN(LOADADDR(.rodata) + SIZEOF(.rodata), 4))
    ALIGN(4)
    {
        *(.data);
    } =0
    __data_rom_start = LOADADDR(.data);
    __data_rom_end = LOADADDR(.data) + SIZEOF(.data);

    /* libc's variables, after the other libraries' (ld/syms.ld). */
    .bss 0x03002428 (NOLOAD) :
    ALIGN(4)
    {
        *(.bss);
        *(COMMON);
    }

    /DISCARD/ :
    {
        *(*);
    }
}
