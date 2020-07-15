
#define ASM 1

.text

.globl syscall_handler

syscall_handler:

    cmpl $10, %eax
    jg error
    cmpl $1, %eax
    jb error
    addl $-1, %eax

    push %edx
    push %ecx
    push %ebx

    call *jumptable(,%eax,4)

    addl $12, %esp
    iret

error:

    movl $-1, %eax
    iret

jumptable:

 .long halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn
