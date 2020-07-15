// #include "syscalls.h"

/*
 * Rather than create a case for each number of arguments, we simplify
 * and use one macro for up to three arguments; the system calls should
 * ignore the other registers, and they're caller-saved anyway.
 */
#define DO_CALL(name,number)   \
.GLOBL name                   ;\
name:   PUSHL	%EBX          ;\
	MOVL	$number,%EAX  ;\
	MOVL	8(%ESP),%EBX  ;\
	MOVL	12(%ESP),%ECX ;\
	MOVL	16(%ESP),%EDX ;\
	INT	$0x80         ;\
	POPL	%EBX          ;\
	RET

/* the system call library wrappers */
DO_CALL(ece391_halt,1)
DO_CALL(ece391_execute,2)
DO_CALL(ece391_read,3)
DO_CALL(ece391_write,4)
DO_CALL(ece391_open,5)
DO_CALL(ece391_close,6)
DO_CALL(ece391_getargs,7)
DO_CALL(ece391_vidmap,8)
DO_CALL(ece391_set_handler,9)
DO_CALL(ece391_sigreturn,10)


/* Call the main() function, then halt with its return value. */

// .GLOBAL _start
// _start:
// 	CALL	main
//     PUSHL   $0
//     PUSHL   $0
// 	PUSHL	%EAX
// 	CALL	ece391_halt

