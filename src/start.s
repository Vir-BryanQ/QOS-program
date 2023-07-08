[bits 32]

extern main
extern exit
global _start

SECTION .text

_start:
    push ebx
    push ecx
    call main
    add esp, 8

    push eax
    call exit