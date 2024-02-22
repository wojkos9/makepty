#include "elf.h"
.code32
.section .text
ehdr:
.byte 0x7F, 'E', 'L', 'F', ELFCLASS32, ELFDATA2LSB, EV_CURRENT, ELFOSABI_NONE
.fill 8
.word ET_EXEC
.word EM_386
.long EV_CURRENT
.long _start
.long phdr - .text
.long 0
.long 0
.word ehsz
.word pesz
.word phsz / pesz
.word 0, 0, 0

.equ ehsz, . - ehdr
phdr:
.long PT_LOAD
.long phdr - .text
.long phdr, phdr
.long fsz, fsz
.long PF_W | PF_X
.long 0x1000
.equ pesz, . - phdr

.equ phsz, . - phdr

_start:
.include "elf_codesize.s"
// mov $42, %bl
// xor %eax, %eax
// inc %eax
// int $0x80
.equ fsz, . - .text + codesize