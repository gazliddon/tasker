/*
 * setstack.s
 * msd
 *
 * Created by Gary Liddon on 15/03/2009.
 * Copyright 2009 Xen. All rights reserved.
 */
.section __TEXT,__text,regular,pure_instructions

	.globl _GSYS_SetStackAndJump
	


#if defined(__x86_64__)
//	; GLIB_API void	GSYS_SetStackAndJump(void *Stack,void(*Func)(void *),void *Param);
//	; %rdi		; Stack
//	; %rsi		; Function
//	; %rdi		; Param


_GSYS_SetStackAndJump:
    push    %rbp
    movq    %rsp,%rax

    movq	%rdi, %rsp
    sub     $0x10,%rsp
    movq    %rax, 2(%rsp)
    movq    %rdx, %rdi

    callq	*%rsi

    movq    2(%rsp), %rsp
    pop     %rbp

_GTIMSYS_GetTimer:
_GTIMSYS_InitTimer:
_GTIMSYS_ResetTimer:
    ret

#endif

#if defined(__i386__)

//	; GLIB_API void	GSYS_SetStackAndJump(void *Stack,void(*Func)(void *),void *Param);
//	; [esp]		; Return Address
//	; [esp+4]	; Stack
//	; [esp+8]	; Fun
//	; [esp+12]	; Param

_GSYS_SetStackAndJump:

	movl 4(%esp),%eax
	movl 8(%esp),%ebx
	movl 12(%esp),%ecx

	
	movl	%eax,%esp

	
	pushl	%ebp				// save stack frame
	movl	%esp,%ebp			// Copy stack to stack frame
	subl	$12,%esp			// Make sure stack is 16 byte aligned for Function call

	movl	%ecx,(%esp)		// param onto the stack
	call	*%ebx				// call the route
	
	addl	12,%esp
	popl	%ebp
	
_GTIMSYS_GetTimer:
_GTIMSYS_InitTimer:
_GTIMSYS_ResetTimer:
	ret

#endif
	
