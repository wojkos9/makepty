OUTPUT_FORMAT(binary)
SECTIONS
{
    . = 0x08048000;
    .text : {
        *(.text)
    }
    /DISCARD/ : {
        *(.*)
    }
}